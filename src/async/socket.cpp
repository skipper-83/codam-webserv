#include "async/socket.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cstring>
#include <logging.hpp>

static CPPLog::Instance logI = logOut.instance(CPPLog::Level::INFO, "Async");
static CPPLog::Instance logD = logOut.instance(CPPLog::Level::DEBUG, "Async");
static CPPLog::Instance logW = logOut.instance(CPPLog::Level::WARNING, "Async");
static CPPLog::Instance logE = logOut.instance(CPPLog::Level::ERROR, "Async");
static CPPLog::Instance logF = logOut.instance(CPPLog::Level::FATAL, "Async");

AsyncSocket::AsyncSocket(uint16_t port, const SocketCallback &clientAvailableCb, int backlog)
    : AsyncFD({{EventTypes::IN, AsyncSocket::_internalClientAvailableCb}}), _port(port), _clientAvailableCb(clientAvailableCb), _backlog(backlog) {
    logD << "AsyncSocket::AsyncSocket(uint16_t, const SocketCallback&, int) called";
    _fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_fd < 0) {
        logE << "AsyncSocket::AsyncSocket(uint16_t, const SocketCallback&, int) failed: " << std::strerror(errno);
        throw std::runtime_error(std::strerror(errno));
    }

    int opt = 1;
    if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0 || setsockopt(_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        logE << "AsyncSocket::AsyncSocket(uint16_t, const SocketCallback&, int) failed: " << std::strerror(errno);
        throw std::runtime_error(std::strerror(errno));
    }
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(_port);

    if (bind(_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        logE << "AsyncSocket::AsyncSocket(uint16_t, const SocketCallback&, int) failed: " << std::strerror(errno);
        throw std::runtime_error(std::strerror(errno));
    }

    if (listen(_fd, _backlog) < 0) {
        logE << "AsyncSocket::AsyncSocket(uint16_t, const SocketCallback&, int) failed: " << std::strerror(errno);
        throw std::runtime_error(std::strerror(errno));
    }
    setAsyncFlags();
}

std::unique_ptr<AsyncSocket> AsyncSocket::create(uint16_t port, const SocketCallback &clientAvailableCb, int backlog) {
    logD << "AsyncSocket::create(uint16_t, const SocketCallback&, int) called";
    return std::make_unique<AsyncSocket>(port, clientAvailableCb, backlog);
}

AsyncSocket::~AsyncSocket() {
    logD << "AsyncSocket::~AsyncSocket() called";
}

std::unique_ptr<AsyncSocketClient> AsyncSocket::accept(const AsyncSocketClient::SocketClientCallback &clientReadReadyCb,
                                                       const AsyncSocketClient::SocketClientCallback &clientWriteReadyCb) {
    logD << "AsyncSocket::accept(const AsyncSocketClient::SocketClientCallback&, const AsyncSocketClient::SocketClientCallback&) called";
    int fd = ::accept(_fd, nullptr, nullptr);
    if (fd < 0)
    {
        logE << "AsyncSocket::accept(const AsyncSocketClient::SocketClientCallback&, const AsyncSocketClient::SocketClientCallback&) failed: " << std::strerror(errno);
        throw std::runtime_error(std::strerror(errno));
    }
    _hasPendingAccept = false;
    return AsyncSocketClient::create(fd, _port, clientReadReadyCb, clientWriteReadyCb);
}

void AsyncSocket::_internalClientAvailableCb(AsyncFD &fd) {
    logD << "AsyncSocket::_internalClientAvailableCb(AsyncFD&) called";
    AsyncSocket &socket = dynamic_cast<AsyncSocket &>(fd);
    socket._hasPendingAccept = true;
    if (socket._clientAvailableCb)
        socket._clientAvailableCb(socket);
}

bool AsyncSocket::clientAvailable() const {
    logD << "AsyncSocket::clientAvailable() called";
    return _hasPendingAccept;
}
