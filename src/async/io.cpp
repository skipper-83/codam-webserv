#include <logging.hpp>

#include "async/io.hpp"

static CPPLog::Instance logI = logOut.instance(CPPLog::Level::INFO, "AsyncFD");
static CPPLog::Instance logD = logOut.instance(CPPLog::Level::DEBUG, "AsyncFD");
static CPPLog::Instance logW = logOut.instance(CPPLog::Level::WARNING, "AsyncFD");
static CPPLog::Instance logE = logOut.instance(CPPLog::Level::ERROR, "AsyncFD");
static CPPLog::Instance logF = logOut.instance(CPPLog::Level::FATAL, "AsyncFD");

AsyncIO::AsyncIO(int fd, const std::map<EventTypes, EventCallback> &eventCallbacks)
    : AsyncInput(fd, eventCallbacks), AsyncOutput(fd, eventCallbacks) {
}

AsyncIO::AsyncIO(const std::map<EventTypes, EventCallback> &eventCallbacks)
    : AsyncInput(eventCallbacks), AsyncOutput(eventCallbacks) {
}

std::unique_ptr<AsyncIO> AsyncIO::create(int fd, const std::map<EventTypes, EventCallback> &eventCallbacks) {
    return std::make_unique<AsyncIO>(fd, eventCallbacks);
}

AsyncIO::~AsyncIO() {}
