#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cstring>

#include "async/socket.hpp"

using namespace std::placeholders;
AsyncSocket::AsyncSocket(uint16_t port, const SocketCallback &clientAvailableCb, int backlog)
    : AsyncFD({{EventTypes::IN, AsyncSocket::_internalClientAvailableCb}}), _port(port), _clientAvailableCb(clientAvailableCb), _backlog(backlog) {
    int fd;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        throw std::runtime_error("socket() failed");

    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0 || setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0)
        throw std::runtime_error("setsockopt() failed: " + std::to_string(errno) + ": " + std::strerror(errno));
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(_port);

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        throw std::runtime_error("bind() failed");

    if (listen(fd, _backlog) < 0)
        throw std::runtime_error("listen() failed");
    _fd = fd;
    setAsyncFlags();
}

std::unique_ptr<AsyncSocket> AsyncSocket::create(uint16_t port, const SocketCallback &clientAvailableCb, int backlog) {
    return std::make_unique<AsyncSocket>(port, clientAvailableCb, backlog);
}

AsyncSocket::~AsyncSocket() {}

std::unique_ptr<AsyncSocketClient> AsyncSocket::accept(const AsyncSocketClient::SocketClientCallback &clientReadReadyCb,
                                                       const AsyncSocketClient::SocketClientCallback &clientWriteReadyCb) {
    int fd = ::accept(_fd, nullptr, nullptr);
    if (fd < 0)
        throw std::runtime_error("accept() failed");
    _hasPendingAccept = false;
    return AsyncSocketClient::create(fd, _port, clientReadReadyCb, clientWriteReadyCb);
}

void AsyncSocket::_internalClientAvailableCb(AsyncFD &fd) {
    AsyncSocket &socket = dynamic_cast<AsyncSocket &>(fd);
    socket._hasPendingAccept = true;
    if (socket._clientAvailableCb)
        socket._clientAvailableCb(socket);
}

bool AsyncSocket::clientAvailable() const {
    return _hasPendingAccept;
}
