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

void initiateSockets(AsyncPollArray& pollArray, std::vector<Client>& clients) {
	for (uint16_t port : mainConfig.getPorts()) {
		mainLogI << "creating socket on port " << port << CPPLog::end;
		std::shared_ptr<AsyncSocket> socket = AsyncSocket::create(port, std::bind(mainClientAvailableCb, std::placeholders::_1, std::ref(pollArray), std::ref(clients)));
		try {
			pollArray.add(socket);
		} catch (const std::exception& e) {
			throw std::runtime_error(std::string("failed to add socket to pollArray: ") + e.what());
		}
	}
}

int main(int argc, char** argv) {
    AsyncPollArray pollArray;
    std::vector<Client> clients;

    try {
        parseConfig(argc, argv);
		initiateSockets(pollArray, clients);
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }

    while (true) {
        pollArray.poll(5);
		// mainLogI << "client array size " << clients.size() << CPPLog::end;
		clients.erase(std::remove_if(clients.begin(), clients.end(), [](const Client& client) { 
			std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
			if (!client.socketFd().isValid()) {
				mainLogI << "removing invalid client" << CPPLog::end;
				client.socketFd().close();
				return true;
			}
			if ((now - client.getLastActivityTime()) > mainConfig._timeOutDuration) {
				mainLogI << "removing client due to inactivity" << CPPLog::end;
				client.socketFd().close();
				return true;
			}
			return false;
			}), clients.end());
		usleep(5000);
    }
}