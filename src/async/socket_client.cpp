#include "async/socket_client.hpp"

#include <logging.hpp>

static CPPLog::Instance logI = logOut.instance(CPPLog::Level::INFO, "Async");
static CPPLog::Instance logD = logOut.instance(CPPLog::Level::DEBUG, "Async");
static CPPLog::Instance logW = logOut.instance(CPPLog::Level::WARNING, "Async");
static CPPLog::Instance logE = logOut.instance(CPPLog::Level::ERROR, "Async");
static CPPLog::Instance logF = logOut.instance(CPPLog::Level::FATAL, "Async");

AsyncSocketClient::AsyncSocketClient(int fd, uint16_t port, const SocketClientCallback &clientReadReadyCb,
                                     const SocketClientCallback &clientWriteReadyCb)
    : AsyncFD(fd, {{EventTypes::IN, _internalReadReadyCb}, {EventTypes::OUT, _internalWriteReadyCb}}),  AsyncIO(fd, {{EventTypes::IN, _internalReadReadyCb}, {EventTypes::OUT, _internalWriteReadyCb}}),
      _port(port),
      _clientReadReadyCb(clientReadReadyCb),
      _clientWriteReadyCb(clientWriteReadyCb) {
    logD << "AsyncSocketClient::AsyncSocketClient(int, uint16_t, const SocketClientCallback&, const SocketClientCallback&) called";
}

std::unique_ptr<AsyncSocketClient> AsyncSocketClient::create(int fd, uint16_t port, const SocketClientCallback &clientReadReadyCb,
                                                             const SocketClientCallback &clientWriteReadyCb) {
    logD << "AsyncSocketClient::create(int, uint16_t, const SocketClientCallback&, const SocketClientCallback&) called";
    return std::make_unique<AsyncSocketClient>(fd, port, clientReadReadyCb, clientWriteReadyCb);
}

AsyncSocketClient::~AsyncSocketClient() {
    logD << "AsyncSocketClient::~AsyncSocketClient() called";
}

uint16_t AsyncSocketClient::getPort() const {
    logD << "AsyncSocketClient::getPort() called";
    return _port;
}

void AsyncSocketClient::_internalReadReadyCb(AsyncFD &fd) {
    logD << "AsyncSocketClient::_internalReadReadyCb(AsyncFD&) called";
    auto &client = dynamic_cast<AsyncSocketClient &>(fd);
    if (client._clientReadReadyCb) {
        client._clientReadReadyCb(client);
    }
}

void AsyncSocketClient::_internalWriteReadyCb(AsyncFD &fd) {
    logD << "AsyncSocketClient::_internalWriteReadyCb(AsyncFD&) called";
    auto &client = dynamic_cast<AsyncSocketClient &>(fd);
    if (client._clientWriteReadyCb) {
        client._clientWriteReadyCb(client);
    }
}

void AsyncSocketClient::registerReadReadyCb(const SocketClientCallback &cb) {
    logD << "AsyncSocketClient::registerReadReadyCb(const SocketClientCallback&) called";
    _clientReadReadyCb = cb;
}

void AsyncSocketClient::registerWriteReadyCb(const SocketClientCallback &cb) {
    logD << "AsyncSocketClient::registerWriteReadyCb(const SocketClientCallback&) called";
    _clientWriteReadyCb = cb;
}
