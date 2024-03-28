#include "async/output.hpp"
#include <unistd.h>
#include <cstring>
#include <logging.hpp>

static CPPLog::Instance logD = logOut.instance(CPPLog::Level::DEBUG, "Async");
static CPPLog::Instance logI = logOut.instance(CPPLog::Level::INFO, "Async");
static CPPLog::Instance logW = logOut.instance(CPPLog::Level::WARNING, "Async");
static CPPLog::Instance logE = logOut.instance(CPPLog::Level::ERROR, "Async");
static CPPLog::Instance logF = logOut.instance(CPPLog::Level::FATAL, "Async");

AsyncOutput::AsyncOutput(int fd, const std::map<EventTypes, EventCallback> &eventCallbacks)
    : AsyncFD(fd, eventCallbacks), _outCb(), _hasPendingWrite(false) {
    logD << "AsyncOutput::AsyncOutput(int, const std::map<EventTypes, EventCallback>&) called";

    if (eventCallbacks.find(EventTypes::OUT) != eventCallbacks.end()) {
        _outCb = eventCallbacks.at(EventTypes::OUT);
    }
    _eventCallbacks[EventTypes::OUT] = AsyncOutput::_internalOutCb;
}

AsyncOutput::AsyncOutput(const std::map<EventTypes, EventCallback> &eventCallbacks) : AsyncFD(eventCallbacks), _outCb(), _hasPendingWrite(false) {
    logD << "AsyncOutput::AsyncOutput(const std::map<EventTypes, EventCallback>&) called";

    if (eventCallbacks.find(EventTypes::OUT) != eventCallbacks.end()) {
        _outCb = eventCallbacks.at(EventTypes::OUT);
    }
    _eventCallbacks[EventTypes::OUT] = AsyncOutput::_internalOutCb;
}

std::unique_ptr<AsyncOutput> AsyncOutput::create(int fd, const std::map<EventTypes, EventCallback> &eventCallbacks) {
    logD << "AsyncOutput::create(int, const std::map<EventTypes, EventCallback>&) called";

    return std::make_unique<AsyncOutput>(fd, eventCallbacks);
}

AsyncOutput::~AsyncOutput() {
    logD << "AsyncOutput::~AsyncOutput() called";
}

size_t AsyncOutput::write(std::string &data) {
    logD << "AsyncOutput::write(const std::string&) called";

    if (!_hasPendingWrite) {
        logW << "AsyncOutput::write(const std::string&) called without pending write";
        return 0;
    }

    if (data.empty()) {
        logW << "AsyncOutput::write(const std::string&) called with empty data";
        return 0;
    }

    _hasPendingWrite = false;

    ssize_t bytesWritten = ::write(_fd, data.data(), data.size());
    if (bytesWritten < 0) {
        logE << "AsyncOutput::write(const std::string&) failed: " << std::strerror(errno);
        throw std::runtime_error(std::strerror(errno));
    }

    return static_cast<size_t>(bytesWritten);
}

bool AsyncOutput::hasPendingWrite() const {
    logD << "AsyncOutput::hasPendingWrite() called";

    return _hasPendingWrite;
}

void AsyncOutput::_internalOutCb(AsyncFD &fd) {
    logD << "AsyncOutput::_internalOutCb(AsyncFD&) called";
    AsyncOutput &output = dynamic_cast<AsyncOutput &>(fd);
    output._hasPendingWrite = true;
    if (output._outCb) {
        output._outCb(output);
    }
}
