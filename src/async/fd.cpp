#include "async/fd.hpp"

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

#include <cstring>
#include <logging.hpp>
#include <stdexcept>

static CPPLog::Instance logI = logOut.instance(CPPLog::Level::INFO, "Async");
static CPPLog::Instance logD = logOut.instance(CPPLog::Level::DEBUG, "Async");
static CPPLog::Instance logW = logOut.instance(CPPLog::Level::WARNING, "Async");
static CPPLog::Instance logE = logOut.instance(CPPLog::Level::ERROR, "Async");
static CPPLog::Instance logF = logOut.instance(CPPLog::Level::FATAL, "Async");

const std::map<int, AsyncFD::EventTypes> AsyncFD::pollToEventType = {
    {POLLIN, EventTypes::IN},
    {POLLOUT, EventTypes::OUT},
    {POLLERR, EventTypes::ERROR},
    {POLLHUP, EventTypes::HANGUP},
};

const std::map<AsyncFD::EventTypes, int> AsyncFD::eventTypeToPoll = {
    {EventTypes::IN, POLLIN},
    {EventTypes::OUT, POLLOUT},
    {EventTypes::ERROR, POLLERR},
    {EventTypes::HANGUP, POLLHUP},
};

AsyncFD::AsyncFD(int fd, const std::map<EventTypes, EventCallback>& eventCallbacks) : _fd(fd), _eventCallbacks(eventCallbacks) {
    logD << "AsyncFD::AsyncFD(int, const std::map<EventTypes, EventCallback>&) called";
    setAsyncFlags();
}

AsyncFD::AsyncFD(const std::map<EventTypes, EventCallback>& eventCallbacks) : _fd(-1), _eventCallbacks(eventCallbacks) {
    logD << "AsyncFD::AsyncFD(const std::map<EventTypes, EventCallback>&) called";
}

AsyncFD::AsyncFD() : _fd(-1) {
    logD << "AsyncFD::AsyncFD() called";
}

std::unique_ptr<AsyncFD> AsyncFD::create(int fd, const std::map<EventTypes, EventCallback>& eventCallbacks) {
    logD << "AsyncFD::create(int, const std::map<EventTypes, EventCallback>&) called";
    return std::make_unique<AsyncFD>(fd, eventCallbacks);
}

std::unique_ptr<AsyncFD> AsyncFD::create(const std::map<EventTypes, EventCallback>& eventCallbacks) {
    logD << "AsyncFD::create(const std::map<EventTypes, EventCallback>&) called";
    return std::make_unique<AsyncFD>(eventCallbacks);
}

AsyncFD::~AsyncFD() {
    logD << "AsyncFD::~AsyncFD() called";
    if (!isValid())
        return;
    try {
        close();
    } catch (std::exception& e) {
        logF << "exception in destructor (ignore): " << e.what();
    }
}

void AsyncFD::close() {
    logD << "AsyncFD::close() called";
    if (_fd < 0) {
        logW << "AsyncFD::close() called on invalid fd";
        return;
    }
    int ret = ::close(_fd);
    _fd = -1;
    if (ret < 0) {
        logF << "AsyncFD::close() close(fd) failed: " << std::strerror(errno);
        throw std::runtime_error(std::strerror(errno));
    }
}

bool AsyncFD::isValid() const {
    // logD << "AsyncFD::isValid() called";
    return _fd >= 0;
}

AsyncFD::operator bool() const {
    logD << "AsyncFD::operator bool() called";
    return isValid();
}

void AsyncFD::setAsyncFlags() {
    logD << "AsyncFD::setAsyncFlags() called";
    int flags = fcntl(_fd, F_GETFL, 0);
    if (flags < 0) {
        logF << "AsyncFD::setAsyncFlags() fcntl(F_GETFL) failed: " << std::strerror(errno);
        throw std::runtime_error(std::strerror(errno));
    }
    if (fcntl(_fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        logF << "AsyncFD::setAsyncFlags() fcntl(F_SETFL, flags | O_NONBLOCK) failed: " << std::strerror(errno);
        throw std::runtime_error(std::strerror(errno));
    }
}

void AsyncFD::poll() {
    logD << "AsyncFD::poll() called";
    struct pollfd pfd;
    pfd.fd = _fd;
    pfd.events = pfd.revents = 0;
    for (auto [type, cb] : _eventCallbacks) {
        pfd.events |= eventTypeToPoll.at(type);
    }
    int ret = ::poll(&pfd, 1, -1);
    if (ret < 0) {
        logF << "AsyncFD::poll() poll failed: " << std::strerror(errno);
        throw std::runtime_error(std::strerror(errno));
    }
    for (auto [pollType, eventType] : pollToEventType) {
        if (pfd.revents & pollType) {
            eventCb(eventType);
        }
    }
}

void AsyncFD::eventCb(EventTypes type) {
    logD << "AsyncFD::eventCb(EventTypes) called";
    if (!_eventCallbacks.contains(type)) {
        logW << "AsyncFD::eventCb(EventTypes) called with unregistered event type";
        if (type == EventTypes::ERROR || type == EventTypes::HANGUP)
            this->close();
        return;
    }
    _eventCallbacks.at(type)(*this);
}