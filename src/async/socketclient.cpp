#include "async/fd.hpp"

AsyncSocketClient::AsyncSocketClient(int fd, uint16_t port, const SocketClientCallback &clientReadReadyCb,
                                     const SocketClientCallback &clientWriteReadyCb)
    : AsyncIO(fd, {{EventTypes::IN, _internalReadReadyCb}, {EventTypes::OUT, _internalWriteReadyCb}}),
      _port(port),
      _clientReadReadyCb(clientReadReadyCb),
      _clientWriteReadyCb(clientWriteReadyCb) {}

std::unique_ptr<AsyncSocketClient> AsyncSocketClient::create(int fd, uint16_t port, const SocketClientCallback &clientReadReadyCb,
                                                             const SocketClientCallback &clientWriteReadyCb) {
    return std::make_unique<AsyncSocketClient>(fd, port, clientReadReadyCb, clientWriteReadyCb);
}

AsyncSocketClient::~AsyncSocketClient() {}

uint16_t AsyncSocketClient::getPort() const {
    return _port;
}

void AsyncSocketClient::_internalReadReadyCb(AsyncFD &fd) {
    auto &client = static_cast<AsyncSocketClient &>(fd);
    if (client._clientReadReadyCb) {
        client._clientReadReadyCb(client);
    }
}

void AsyncSocketClient::_internalWriteReadyCb(AsyncFD &fd) {
    auto &client = static_cast<AsyncSocketClient &>(fd);
    if (client._clientWriteReadyCb) {
        client._clientWriteReadyCb(client);
    }
}

void AsyncSocketClient::registerReadReadyCb(const SocketClientCallback &cb) {
    _clientReadReadyCb = cb;
}

void AsyncSocketClient::registerWriteReadyCb(const SocketClientCallback &cb) {
    _clientWriteReadyCb = cb;
}
