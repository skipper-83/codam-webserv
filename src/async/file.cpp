#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <logging.hpp>
#include <stdexcept>

#include "async/fd.hpp"

static CPPLog::Instance logI = logOut.instance(CPPLog::Level::INFO, "AsyncFD");
static CPPLog::Instance logD = logOut.instance(CPPLog::Level::DEBUG, "AsyncFD");
static CPPLog::Instance logW = logOut.instance(CPPLog::Level::WARNING, "AsyncFD");
static CPPLog::Instance logE = logOut.instance(CPPLog::Level::ERROR, "AsyncFD");
static CPPLog::Instance logF = logOut.instance(CPPLog::Level::FATAL, "AsyncFD");

AsyncFile::AsyncFile(const std::string &path, const AsyncFileCallback &clientReadReadyCb)
    : AsyncIO({{EventTypes::IN, AsyncFile::_internalInCb}}), _clientReadReadyCb(clientReadReadyCb), _path(path) {
    _fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
	logI << "AsyncFile: opened file: " << path << " fd: " << _fd << CPPLog::end;
    if (_fd < 0) {
        throw std::runtime_error("failed to open file: " + path + " " + strerror(errno) + "fd: " + std::to_string(_fd) + "errono: " + std::to_string(errno));
    }
}

std::unique_ptr<AsyncFile> AsyncFile::create(const std::string &path, const AsyncFileCallback &clientReadReadyCb) {
	return std::make_unique<AsyncFile>(path, clientReadReadyCb);
}

AsyncFile::~AsyncFile() {}

void AsyncFile::_internalInCb(AsyncFD &fd) {
	AsyncFile &file = static_cast<AsyncFile &>(fd);
	logI << "AsyncFile: read ready cb" << CPPLog::end;
	file._hasPendingRead = true;
	if (file._clientReadReadyCb)
		file._clientReadReadyCb(file);
}