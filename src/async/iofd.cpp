#include <logging.hpp>

#include "async/fd.hpp"

static CPPLog::Instance logI = logOut.instance(CPPLog::Level::INFO, "AsyncFD");
static CPPLog::Instance logD = logOut.instance(CPPLog::Level::DEBUG, "AsyncFD");
static CPPLog::Instance logW = logOut.instance(CPPLog::Level::WARNING, "AsyncFD");
static CPPLog::Instance logE = logOut.instance(CPPLog::Level::ERROR, "AsyncFD");
static CPPLog::Instance logF = logOut.instance(CPPLog::Level::FATAL, "AsyncFD");

AsyncIO::AsyncIO(int fd, const std::map<EventTypes, EventCallback> &eventCallbacks)
    : AsyncFD(fd, eventCallbacks), _inCb(), _outCb(), _hasPendingRead(false), _hasPendingWrite(false), _eof(false) {
    if (_eventCallbacks.contains(EventTypes::IN))
        _inCb = _eventCallbacks.at(EventTypes::IN);
    if (_eventCallbacks.contains(EventTypes::OUT))
        _outCb = _eventCallbacks.at(EventTypes::OUT);

    _eventCallbacks[EventTypes::IN] = AsyncIO::_internalInCb;
    _eventCallbacks[EventTypes::OUT] = AsyncIO::_internalOutReadyCb;
}

AsyncIO::AsyncIO(const std::map<EventTypes, EventCallback> &eventCallbacks)
    : AsyncFD(eventCallbacks), _inCb(), _outCb(), _hasPendingRead(false), _hasPendingWrite(false), _eof(false) {
    if (_eventCallbacks.contains(EventTypes::IN))
        _inCb = _eventCallbacks.at(EventTypes::IN);
    if (_eventCallbacks.contains(EventTypes::OUT))
        _outCb = _eventCallbacks.at(EventTypes::OUT);

    _eventCallbacks[EventTypes::IN] = AsyncIO::_internalInCb;
    _eventCallbacks[EventTypes::OUT] = AsyncIO::_internalOutReadyCb;
}

std::unique_ptr<AsyncIO> AsyncIO::create(int fd, const std::map<EventTypes, EventCallback> &eventCallbacks) {
    return std::make_unique<AsyncIO>(fd, eventCallbacks);
}

AsyncIO::~AsyncIO() {}

void AsyncIO::_internalInCb(AsyncFD &fd) {
    AsyncIO &io = static_cast<AsyncIO &>(fd);
    if (!io._hasPendingRead)
        logI << "read ready cb";
    io._hasPendingRead = true;
    if (io._inCb)
        io._inCb(io);
}

void AsyncIO::_internalOutReadyCb(AsyncFD &fd) {
    AsyncIO &io = static_cast<AsyncIO &>(fd);
    if (!io._hasPendingWrite)
        logI << "write ready cb";
    io._hasPendingWrite = true;
    if (io._outCb)
        io._outCb(io);
}

std::string AsyncIO::read(size_t size) {
    std::string ret;

    if (!_hasPendingRead) {
        logW << "no pending read";
        return ret;
    }
    _hasPendingRead = false;
    if (size == 0) {
        logW << "read size is 0";
        return ret;
    }
    std::unique_ptr<char[]> buf = std::make_unique<char[]>(size);
    ssize_t retSize = ::read(_fd, buf.get(), size);
    if (retSize < 0) {
        logE << "read failed";
        throw std::runtime_error("read failed");
    }
    if (retSize == 0) {
        logI << "read EOF";
        _eof = true;
        return ret;
    }
    ret.append(buf.get(), retSize);
    return ret;
}

size_t AsyncIO::write(std::string &data) {
    if (!_hasPendingWrite) {
        logW << "no pending write";
        return 0;
    }
    _hasPendingWrite = false;
    if (data.size() == 0) {
        logW << "write size is 0";
        return 0;
    }
    ssize_t ret = ::write(_fd, data.c_str(), data.size());
    if (ret < 0) {
        logE << "write failed";
        throw std::runtime_error("write failed");
    }
    data.erase(0, ret);
    return ret;
}

bool AsyncIO::eof() const {
    return _eof;
}

bool AsyncIO::hasPendingRead() const {
    return _hasPendingRead;
}

bool AsyncIO::hasPendingWrite() const {
    return _hasPendingWrite;
}