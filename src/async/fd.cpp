#include "async/fd.hpp"

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

#include <logging.hpp>
#include <stdexcept>

static CPPLog::Instance logI = logOut.instance(CPPLog::INFO, "AsyncFD");
static CPPLog::Instance logD = logOut.instance(CPPLog::DEBUG, "AsyncFD");
static CPPLog::Instance logW = logOut.instance(CPPLog::WARNING, "AsyncFD");
static CPPLog::Instance logE = logOut.instance(CPPLog::ERROR, "AsyncFD");
static CPPLog::Instance logF = logOut.instance(CPPLog::FATAL, "AsyncFD");

AsyncFD::AsyncFD(int fd) : _fd(fd) {
    setAsyncFlags();
}

AsyncFD::AsyncFD() : _fd(-1) {}

AsyncFD::~AsyncFD() {
    try {
        close();
    } catch (std::exception& e) {
        logF << "exception in destructor (ignore): " << e.what();
    }
}

void AsyncFD::close() {
    if (_fd < 0) {
        logI << "invalid fd";
        return;
    }
    int ret = ::close(_fd);
    _fd = -1;
    if (ret < 0) {
        logF << "close failed";
        throw std::runtime_error("close failed");
    }
}

bool AsyncFD::isValid() const {
    return _fd >= 0;
}

AsyncFD::operator bool() const {
    return isValid();
}

void AsyncFD::setAsyncFlags() {
    int flags = fcntl(_fd, F_GETFL, 0);
    if (flags < 0) {
        logF << "fcntl(F_GETFL) failed";
        throw std::runtime_error("fcntl(F_GETFL) failed");
    }
    flags |= O_NONBLOCK;
    if (fcntl(_fd, F_SETFL, flags) < 0) {
        logF << "fcntl(F_SETFL) failed";
        throw std::runtime_error("fcntl(F_SETFL) failed");
    }
}

void AsyncFD::poll() {
    struct pollfd pfd;
    pfd.fd = _fd;
    pfd.events = POLLIN | POLLOUT | POLLERR;
    pfd.revents = 0;
    int ret = ::poll(&pfd, 1, -1);
    if (ret < 0) {
        logF << "poll failed";
        throw std::runtime_error("poll failed");
    }
    if (pfd.revents & POLLIN)
        readReadyCb();
    if (pfd.revents & POLLOUT)
        writeReadyCb();
    if (pfd.revents & POLLERR)
        errorCb();
}
