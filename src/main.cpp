#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

#include "async/socket.hpp"
#include "async/socket_client.hpp"
#include "async/pollarray.hpp"
#include "client.hpp"
#include "config.hpp"
#include "logging.hpp"

CPPLog::Instance mainLogI = logOut.instance(CPPLog::Level::INFO, "main");
CPPLog::Instance mainLogW = logOut.instance(CPPLog::Level::WARNING, "main");

MainConfig mainConfig;

void mainClientAvailableCb(AsyncSocket& socket, AsyncPollArray& pollArray, std::vector<Client>& clients) {
    mainLogI << "client available CB" << CPPLog::end;
    mainLogI << "creating client" << CPPLog::end;
    std::shared_ptr<AsyncSocketClient> newSocketClient = socket.accept();
    clients.emplace_back(newSocketClient);
    mainLogI << "adding client to pollArray" << CPPLog::end;
    pollArray.add(newSocketClient);
    // usleep(5000);
}

void parseConfig(int argc, char** argv) {
    if (argc != 2)
        throw std::runtime_error(std::string("invalid number of arguments. \nUsage: ") + argv[0] + std::string(" <config file>"));

    std::fstream file(argv[1]);

    if (!file)
        throw std::runtime_error(std::string("failed to open config file: ") + argv[1]);

    try {
        file >> mainConfig;
    } catch (const std::exception& e) {
        logOut.stream(CPPLog::Level::FATAL, "parseConfig") << e.what();
        throw std::runtime_error(std::string("failed to parse config file: ") + argv[1] + std::string(". \nError message: ") + e.what());
    }
}

// TODO: Catch exceptions from AsyncSocket::create
void initiateSockets(AsyncPollArray& pollArray, std::vector<Client>& clients, std::vector<std::shared_ptr<AsyncSocket>>& sockets) {
    for (uint16_t port : mainConfig.getPorts()) {
        mainLogI << "creating socket on port " << port << CPPLog::end;
        std::shared_ptr<AsyncSocket> socket =
            AsyncSocket::create(port, std::bind(mainClientAvailableCb, std::placeholders::_1, std::ref(pollArray), std::ref(clients)));
        sockets.push_back(socket);
        try {
            pollArray.add(socket);
        } catch (const std::exception& e) {
            throw std::runtime_error(std::string("failed to add socket to pollArray: ") + e.what());
        }
    }
}

#include "file_handler.hpp"

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    AsyncPollArray pollArray;

    InFileHandler fileHandler("test.txt", 1024);

    pollArray.add((std::shared_ptr<AsyncFD>)fileHandler);

    while (true) {
        pollArray.poll(5);

        if (!fileHandler.readBufferEmpty()) {
            std::string data = fileHandler.read();
            std::cout << "Read: " << data << std::endl;
        }
    }
}