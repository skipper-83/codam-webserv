#include "Socket.hpp"

#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <stdexcept>

Socket::Socket(int port, int backlog) : FileDescriptor() {
    this->_socket();
    this->_fcntl();
    this->_bind(port);
    this->_listen(backlog);
}

Socket::~Socket() {}

std::unique_ptr<Connection> Socket::accept() {
    int connection_fd = ::accept(this->getFD(), NULL, NULL);
    if (connection_fd < 0) {
        throw std::runtime_error("Failed to accept connection");
    }
    return std::make_unique<Connection>(connection_fd);
}

bool Socket::pending() const {
    static const int fd_count = 1;
    struct pollfd fds[fd_count];

    fds[0].fd = this->getFD();
    fds[0].events = POLLIN;

    return (poll(fds, fd_count, 0) > 0);
}

void Socket::_socket() {
    try {
        this->setFD(::socket(AF_INET, SOCK_STREAM, 0));
    } catch (const std::runtime_error &e) {
        throw std::runtime_error("Failed to create socket");
    }
}

void Socket::_bind(int port) {
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(this->getFD(), (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        throw std::runtime_error("Failed to bind socket");
    }
}

void Socket::_listen(int backlog) {
    if (listen(this->getFD(), backlog) < 0) {
        throw std::runtime_error("Failed to listen on socket");
    }
}

void Socket::_fcntl() {
    int flags = ::fcntl(this->getFD(), F_GETFL, 0);
    if (flags < 0) {
        throw std::runtime_error("Failed to get socket flags");
    }
    if (::fcntl(this->getFD(), F_SETFL, flags | O_NONBLOCK) < 0) {
        throw std::runtime_error("Failed to set socket flags");
    }
}