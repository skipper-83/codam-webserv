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
				client.clientReadCb();
            

        });
    }
}