#include <logging.hpp>

#include "async/fd.hpp"

static CPPLog::Instance logI = logOut.instance(CPPLog::Level::INFO, "AsyncFD");
static CPPLog::Instance logD = logOut.instance(CPPLog::Level::DEBUG, "AsyncFD");
static CPPLog::Instance logW = logOut.instance(CPPLog::Level::WARNING, "AsyncFD");
static CPPLog::Instance logE = logOut.instance(CPPLog::Level::ERROR, "AsyncFD");
static CPPLog::Instance logF = logOut.instance(CPPLog::Level::FATAL, "AsyncFD");

AsyncIOFD::AsyncIOFD(int fd) : AsyncFD(fd) {}

std::unique_ptr<AsyncIOFD> AsyncIOFD::create(int fd) {
    return std::make_unique<AsyncIOFD>(fd);
}

AsyncIOFD::~AsyncIOFD() {}

void AsyncIOFD::readReadyCb() {
    std::unique_ptr<char[]> buf = std::make_unique<char[]>(READ_CHUNK_SIZE);
    ssize_t ret = ::read(this->_fd, buf.get(), READ_CHUNK_SIZE);
    if (ret < 0) {
        logE << "read failed";
        throw std::runtime_error("read failed");
    }
    if (ret == 0) {
        logI << "read EOF";
        close();
        return;
    }
    readBuffer.append(buf.get(), ret);
}

void AsyncIOFD::writeReadyCb() {
    if (writeBuffer.size() == 0)
        return;
    if (!*this) {
        logI << "invalid fd";
        return;
    }
    ssize_t ret = ::write(this->_fd, writeBuffer.c_str(), writeBuffer.size());
    if (ret < 0) {
        logE << "write failed";
        throw std::runtime_error("write failed");
    }
    writeBuffer.erase(0, ret);
}

void AsyncIOFD::errorCb() {}