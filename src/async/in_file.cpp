#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

#include <logging.hpp>
#include <stdexcept>
#include <cstring>

#include "async/in_file.hpp"

static CPPLog::Instance logI = logOut.instance(CPPLog::Level::INFO, "Async");
static CPPLog::Instance logD = logOut.instance(CPPLog::Level::DEBUG, "Async");
static CPPLog::Instance logW = logOut.instance(CPPLog::Level::WARNING, "Async");
static CPPLog::Instance logE = logOut.instance(CPPLog::Level::ERROR, "Async");
static CPPLog::Instance logF = logOut.instance(CPPLog::Level::FATAL, "Async");

AsyncInFile::AsyncInFile(const std::string& path, const AsyncInFileCallback& cb) : AsyncInput({{EventTypes::IN, _internalInCb}}), _inCb(cb) {
    logD << "AsyncInFile::AsyncInFile(const std::string&, const AsyncInFileCallback&) called";
    _fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
    if (_fd < 0) {
        std::string err = "AsyncInFile::AsyncInFile(const std::string&, const AsyncInFileCallback&) failed: " + std::string(std::strerror(errno)) + " file: " + path;
        throw std::runtime_error("failed to open file");
    }
    setAsyncFlags();
}

std::unique_ptr<AsyncInFile> AsyncInFile::create(const std::string& path, const AsyncInFileCallback& cb) {
    logD << "AsyncInFile::create(const std::string&, const AsyncInFileCallback&) called";
    return std::make_unique<AsyncInFile>(path, cb);
}

AsyncInFile::~AsyncInFile() {
    logD << "AsyncInFile::~AsyncInFile() called";
}

void AsyncInFile::_internalInCb(AsyncFD& fd) {
    logD << "AsyncInFile::_internalInCb(AsyncFD&) called";
    AsyncInFile& file = dynamic_cast<AsyncInFile&>(fd);
    file._hasPendingRead = true;
    if (file._inCb) {
        file._inCb(file);
    }
}
