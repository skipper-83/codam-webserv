#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

#include "async/fd.hpp"
#include "async/pollarray.hpp"
#include "client.hpp"
#include "config.hpp"
#include "logging.hpp"

CPPLog::Instance mainLogI = logOut.instance(CPPLog::Level::INFO, "main");
CPPLog::Instance mainLogW = logOut.instance(CPPLog::Level::WARNING, "main");

MainConfig mainConfig;

void parseConfig(const std::string& path) {
    std::fstream file(path);

    if (!file)
        throw std::runtime_error("failed to open config file");
    try {
        file >> mainConfig;
    } catch (const std::exception& e) {
        logOut.stream(CPPLog::Level::FATAL, "parseConfig") << e.what();
        throw std::runtime_error("failed to parse config file");
    }
}

int main(int argc, char** argv) {
    mainLogI << "main() called" << CPPLog::end;

    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " <config file>" << std::endl;
        return 1;
    }

    parseConfig(argv[1]);
    std::vector<uint16_t> ports = mainConfig.getPorts();
    std::vector<std::shared_ptr<AsyncSocket>> sockets;
    std::transform(ports.begin(), ports.end(), std::back_inserter(sockets), [](auto& port) { return AsyncSocket::create(port); });
    AsyncPollArray pollArray;
    std::for_each(sockets.begin(), sockets.end(), [&pollArray](auto& socket) { pollArray.add(socket); });

    std::vector<Client> clients;
    while (true) {
        pollArray.poll(-1);
        std::for_each(sockets.begin(), sockets.end(), [&clients, &pollArray](std::shared_ptr<AsyncSocket>& socket) {
            if (socket->hasPendingAccept()) {
                std::shared_ptr<AsyncIOFD> clientSocket = socket->accept();
                uint16_t port = socket->getPort();
                clients.emplace_back(clientSocket, port);
                pollArray.add(clientSocket);
            }
        });

        clients.erase(std::remove_if(clients.begin(), clients.end(),
                                     [](Client& client) {
                                         if (client.fd().isValid())
                                             return false;
                                         mainLogI << "client on port " << client.port() << " disconnected" << CPPLog::end;
                                         return true;
                                     }),
                      clients.end());

        std::for_each(clients.begin(), clients.end(), [](Client& client) {
            if (!client.fd().hasPendingRead)
                return;
            mainLogI << "received " << client.fd().readBuffer.size() << " bytes from " << client.port() << CPPLog::end;
            mainLogI << "received: " << client.fd().readBuffer << CPPLog::end;
            try {
                client.request().parse(client.fd().readBuffer, client.port());
            } catch (const httpRequest::httpRequestException& e) {
                // client.response().setPrecedingRequest(&client.request());
                client.response().setCode(e.errorNo());
                client.fd().writeBuffer = client.response().getResponseAsString();
                mainLogW << "HTTP error " << e.errorNo() << ": " << e.codeDescription() << "\n" << e.what();
                client.fd().readBuffer.clear();
                client.request().clear();
            }

            client.fd().hasPendingRead = false;
            mainLogI << "parsed request from " << client.port() << CPPLog::end;

            if (client.request().headerComplete()) {
                mainLogI << "request header complete" << CPPLog::end;
                mainLogI << "request method: " << client.request().getRequestType() << CPPLog::end;
                mainLogI << "request path: " << client.request().getAdress() << CPPLog::end;
            }

            if (client.request().bodyComplete()) {
                mainLogI << "request body complete" << CPPLog::end;
                mainLogI << "Body: [" << client.request().getBody() << "] " << CPPLog::end;
                client.fd().writeBuffer = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\ntestr\r\n\r\n";
                client.request().clear();
                mainLogI << "body complete part done" << CPPLog::end;
            }
        });
    }
}