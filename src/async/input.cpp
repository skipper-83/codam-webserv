#include "async/input.hpp"

#include <cstring>
#include <logging.hpp>

static CPPLog::Instance logD = logOut.instance(CPPLog::Level::DEBUG, "Async");
static CPPLog::Instance logI = logOut.instance(CPPLog::Level::INFO, "Async");
static CPPLog::Instance logW = logOut.instance(CPPLog::Level::WARNING, "Async");
static CPPLog::Instance logE = logOut.instance(CPPLog::Level::ERROR, "Async");
static CPPLog::Instance logF = logOut.instance(CPPLog::Level::FATAL, "Async");

AsyncInput::AsyncInput(int fd, const std::map<EventTypes, EventCallback> &eventCallbacks)
    : AsyncFD(fd, eventCallbacks), _inCb(), _hasPendingRead(false), _eof(false) {
    logD << "AsyncInput::AsyncInput(int, const std::map<EventTypes, EventCallback>&) called";

    if (eventCallbacks.find(EventTypes::IN) != eventCallbacks.end()) {
        _inCb = eventCallbacks.at(EventTypes::IN);
    }
    _eventCallbacks[EventTypes::IN] = AsyncInput::_internalInCb;
}

AsyncInput::AsyncInput(const std::map<EventTypes, EventCallback> &eventCallbacks)
    : AsyncFD(eventCallbacks), _inCb(), _hasPendingRead(false), _eof(false) {
    logD << "AsyncInput::AsyncInput(const std::map<EventTypes, EventCallback>&) called";

    if (eventCallbacks.find(EventTypes::IN) != eventCallbacks.end()) {
        _inCb = eventCallbacks.at(EventTypes::IN);
    }
    _eventCallbacks[EventTypes::IN] = AsyncInput::_internalInCb;
}

std::unique_ptr<AsyncInput> AsyncInput::create(int fd, const std::map<EventTypes, EventCallback> &eventCallbacks) {
    logD << "AsyncInput::create(int, const std::map<EventTypes, EventCallback>&) called";

    return std::make_unique<AsyncInput>(fd, eventCallbacks);
}

AsyncInput::~AsyncInput() {
    logD << "AsyncInput::~AsyncInput() called";
}

std::string AsyncInput::read(size_t size) {
    logD << "AsyncInput::read(size_t) called";

    if (!_hasPendingRead) {
        logW << "AsyncInput::read(size_t) called without pending read";
        return "";
    }
    if (_eof) {
        logW << "AsyncInput::read(size_t) called after EOF";
        return "";
    }

    _hasPendingRead = false;

    std::string ret;
    ret.resize(size);

    ssize_t bytesRead = ::read(_fd, ret.data(), size);
    if (bytesRead < 0) {
        logE << "AsyncInput::read(size_t) failed: " << std::strerror(errno);
        throw std::runtime_error(std::strerror(errno));
        return "";
    }

    if (bytesRead == 0) {
        _eof = true;
        logI << "AsyncInput::read(size_t) reached EOF";
        return "";
    }
    ret.resize(static_cast<size_t>(bytesRead));
    return ret;
}

bool AsyncInput::hasPendingRead() const {
    logD << "AsyncInput::hasPendingRead() called";
    return _hasPendingRead;
}

void AsyncInput::_internalInCb(AsyncFD &fd) {
    logD << "AsyncInput::_internalInCb(AsyncFD&) called";
    AsyncInput &input = dynamic_cast<AsyncInput &>(fd);
    input._hasPendingRead = true;
    if (input._inCb) {
        input._inCb(input);
    }
}

bool AsyncInput::eof() const {
    logD << "AsyncInput::eof() called";
    return _eof;
}
