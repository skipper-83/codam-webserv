#include "server.hpp"

#include <FDQueue.hpp>
#include <Socket.hpp>

#include "logging.hpp"

FDQueue fdQueue;

CPPLog::Instance serverLogI = logOut.instance(CPPLog::INFO, "server");
CPPLog::Instance serverLogD = logOut.instance(CPPLog::DEBUG, "server");

void connectionRW(FileDescriptor &fd, FDQueue::ListenMode mode) {
    Connection &connection = dynamic_cast<Connection &>(fd);
    if (mode == FDQueue::ListenMode::READ) {
        serverLogD << "Connection read available";
        std::string data = connection.read(1024);
        if (data.empty()) {
            serverLogD << "Connection closed";
            fdQueue.pop(fd, FDQueue::ListenMode::READ | FDQueue::ListenMode::WRITE);
            return;
        }
        serverLogD << "Read data: " << data;
    }
}

void acceptSocket(FileDescriptor &fd, FDQueue::ListenMode) {
    Socket &socket = dynamic_cast<Socket &>(fd);
    serverLogD << "Socket connection available";
    std::unique_ptr<Connection> connection = socket.accept();
    serverLogD << "Connection accepted";
    fdQueue.push(std::move(connection), connectionRW, FDQueue::ListenMode::READ | FDQueue::ListenMode::WRITE);
    serverLogD << "Connection pushed";
}

void server(int port) {
    serverLogI << "Listening on port " << port << CPPLog::end;
    std::unique_ptr<Socket> socket = std::make_unique<Socket>(port);
    fdQueue.push(std::move(socket), acceptSocket, FDQueue::ListenMode::READ);
    serverLogI << "Server started";
    while (true) {
        fdQueue.listen(nullptr);
    }
}