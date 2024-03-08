#include "async/fd.hpp"

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

#include <logging.hpp>
#include <stdexcept>

static CPPLog::Instance logI = logOut.instance(CPPLog::Level::INFO, "AsyncFD");
static CPPLog::Instance logD = logOut.instance(CPPLog::Level::DEBUG, "AsyncFD");
static CPPLog::Instance logW = logOut.instance(CPPLog::Level::WARNING, "AsyncFD");
static CPPLog::Instance logE = logOut.instance(CPPLog::Level::ERROR, "AsyncFD");
static CPPLog::Instance logF = logOut.instance(CPPLog::Level::FATAL, "AsyncFD");

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
    setAsyncFlags();
}

AsyncFD::AsyncFD(const std::map<EventTypes, EventCallback>& eventCallbacks) : _fd(-1), _eventCallbacks(eventCallbacks) {}

AsyncFD::AsyncFD() : _fd(-1) {}

std::unique_ptr<AsyncFD> AsyncFD::create(int fd, const std::map<EventTypes, EventCallback>& eventCallbacks) {
    return std::make_unique<AsyncFD>(fd, eventCallbacks);
}

std::unique_ptr<AsyncFD> AsyncFD::create(const std::map<EventTypes, EventCallback>& eventCallbacks) {
    return std::make_unique<AsyncFD>(eventCallbacks);
}

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
    pfd.events = pfd.revents = 0;
    for (auto [type, cb] : _eventCallbacks) {
        pfd.events |= eventTypeToPoll.at(type);
    }
    logD << "polling for events: " << pfd.events;
    int ret = ::poll(&pfd, 1, -1);
    if (ret < 0) {
        logF << "poll failed";
        throw std::runtime_error("poll failed");
    }

    logD << "poll returned: " << ret;

    for (auto [pollType, eventType] : pollToEventType) {
        if (pfd.revents & pollType) {
            logD << "event: " << pollType;
            eventCb(eventType);
        }
    }
}

void AsyncFD::eventCb(EventTypes type) {
    if (!_eventCallbacks.contains(type)) {
        logW << "no callback for event: " << static_cast<int>(type);
        return;
    }
    _eventCallbacks.at(type)(*this);
}