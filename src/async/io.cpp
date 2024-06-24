#include "async/io.hpp"

#include <logging.hpp>

static CPPLog::Instance logI = logOut.instance(CPPLog::Level::INFO, "Async");
static CPPLog::Instance logD = logOut.instance(CPPLog::Level::DEBUG, "Async");
static CPPLog::Instance logW = logOut.instance(CPPLog::Level::WARNING, "Async");
static CPPLog::Instance logE = logOut.instance(CPPLog::Level::ERROR, "Async");
static CPPLog::Instance logF = logOut.instance(CPPLog::Level::FATAL, "Async");

AsyncIO::AsyncIO(int fd, const std::map<EventTypes, EventCallback> &eventCallbacks)
    : AsyncFD(fd, eventCallbacks), AsyncInput(fd, eventCallbacks), AsyncOutput(fd, eventCallbacks) {
    logD << "AsyncIO::AsyncIO(int, const std::map<EventTypes, EventCallback>&) called";
}

AsyncIO::AsyncIO(const std::map<EventTypes, EventCallback> &eventCallbacks)
    : AsyncFD(eventCallbacks), AsyncInput(eventCallbacks), AsyncOutput(eventCallbacks) {
    logD << "AsyncIO::AsyncIO(const std::map<EventTypes, EventCallback>&) called";
}

std::unique_ptr<AsyncIO> AsyncIO::create(int fd, const std::map<EventTypes, EventCallback> &eventCallbacks) {
    logD << "AsyncIO::create(int, const std::map<EventTypes, EventCallback>&) called";
    return std::make_unique<AsyncIO>(fd, eventCallbacks);
}

AsyncIO::~AsyncIO() {
    logD << "AsyncIO::~AsyncIO() called";
}
