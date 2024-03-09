#include <algorithm>
#include <fstream>
#include <functional>
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
    (void)argc;
    (void)argv;
	std::vector<Client> clients;

    AsyncPollArray pollArray;

    std::function mainClientAvailableCb = [&pollArray, &clients](AsyncSocket& socket) {
        mainLogI << "client available CB" << CPPLog::end;
        mainLogI << "creating client" << CPPLog::end;
        std::shared_ptr<AsyncSocketClient> newSocketClient = socket.accept();
		clients.emplace_back(newSocketClient);
        mainLogI << "adding client to pollArray" << CPPLog::end;
        pollArray.add(newSocketClient);
    };

    std::shared_ptr<AsyncSocket> socket = AsyncSocket::create(8080, mainClientAvailableCb);

    mainLogI << "adding socket to pollArray" << CPPLog::end;
    pollArray.add(socket);

    while (true) {
        pollArray.poll(-1);
    }
}