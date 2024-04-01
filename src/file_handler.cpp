#include "file_handler.hpp"

#include "logging.hpp"

static CPPLog::Instance logD = logOut.instance(CPPLog::Level::DEBUG, "FileHandler");
static CPPLog::Instance logI = logOut.instance(CPPLog::Level::INFO, "FileHandler");
static CPPLog::Instance logW = logOut.instance(CPPLog::Level::WARNING, "FileHandler");
static CPPLog::Instance logE = logOut.instance(CPPLog::Level::ERROR, "FileHandler");
static CPPLog::Instance logF = logOut.instance(CPPLog::Level::FATAL, "FileHandler");

InFileHandler::InFileHandler(std::string path, std::size_t bufferSize) : _bufferSize(bufferSize), _bad(false) {
    logD << "InFileHandler::InFileHandler(std::string, std::size_t) called";
    try {
        _fd = AsyncInFile::create(path, std::bind(&InFileHandler::_readCb, this, std::placeholders::_1));
    } catch (const std::exception& e) {
        logF << "InFileHandler::InFileHandler(std::string, std::size_t) failed: " << e.what() << " errno: " << errno;
        _bad = true;
		_badCode = errno;
    }
}

InFileHandler::~InFileHandler() {
    logD << "InFileHandler::~InFileHandler() called";
}

std::string InFileHandler::read(std::size_t size) {
    logD << "InFileHandler::read(std::size_t) called";
    if (_bad)
        throw std::runtime_error("InFileHandler::read(std::size_t) failed: bad file handler");

    if (size > _bufferSize)
        size = _bufferSize;

    if (size == 0)
        return "";

    std::string ret = _readbuffer.substr(0, size);
    _readbuffer.erase(0, size);
    return ret;
}

bool InFileHandler::readBufferEmpty() const {
    logD << "InFileHandler::readBufferEmpty() called";
    return _readbuffer.empty();
}

std::size_t InFileHandler::readBufferLength() const {
    logD << "InFileHandler::readBufferLength() called";
    return _readbuffer.length();
}

bool InFileHandler::readBufferFull() const {
    logD << "InFileHandler::readBufferFull() called: " << _readbuffer.length() << " >= " << _bufferSize << " = " << (_readbuffer.length() >= _bufferSize);
    return _readbuffer.length() >= _bufferSize;
}

std::size_t InFileHandler::readBufferSpaceLeft() const {
    logD << "InFileHandler::readBufferSpaceLeft() called";
    return _bufferSize - _readbuffer.length();
}

void InFileHandler::_readCb(AsyncInFile& file) {
    logD << "InFileHandler::_readCb(AsyncFD&) called";
    logD << "reading from file";

    try {
        _readbuffer += file.read(_bufferSize - _readbuffer.length());
    } catch (const std::exception& e) {
        logE << "InFileHandler::_readCb(AsyncFD&) failed: " << e.what();
        _bad = true;
    }
    if (file.eof()) {
        logI << "InFileHandler::_readCb(AsyncFD&) reached EOF";
        _eof = true;
        _fd = nullptr;
    }
}

InFileHandler::operator std::shared_ptr<AsyncFD>() const {
    logD << "InFileHandler::operator std::shared_ptr<AsyncFD>() const called";
    return _fd;
}

bool InFileHandler::eof() const {
    logD << "InFileHandler::eof() called";
    return _eof;
}

bool InFileHandler::bad() const {
    logD << "InFileHandler::bad() called";
    return _bad;
}

int InFileHandler::badCode() const {
    return _badCode;
}
