#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "async/fd.hpp"

AsyncSocket::AsyncSocket(uint16_t port, int backlog) : _port(port), _backlog(backlog) {
    int fd;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        throw std::runtime_error("socket() failed");

    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0)
        throw std::runtime_error("setsockopt() failed");

    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        throw std::runtime_error("bind() failed");

    if (listen(fd, backlog) < 0)
        throw std::runtime_error("listen() failed");
    _fd = fd;
    setAsyncFlags();
}

AsyncSocket::~AsyncSocket() {}

void AsyncSocket::readReadyCb() {
    _hasPendingAccept = true;
}

bool AsyncSocket::hasPendingAccept() const {
    return _hasPendingAccept;
}

void AsyncSocket::writeReadyCb() {}

std::unique_ptr<AsyncIOFD> AsyncSocket::accept() {
    int fd = ::accept(_fd, nullptr, nullptr);
    if (fd < 0)
        throw std::runtime_error("accept() failed");
    _hasPendingAccept = false;
    return AsyncIOFD::create(fd);
}

void AsyncSocket::errorCb() {}

std::unique_ptr<AsyncSocket> AsyncSocket::create(uint16_t port, int backlog) {
    return std::make_unique<AsyncSocket>(port, backlog);
}
