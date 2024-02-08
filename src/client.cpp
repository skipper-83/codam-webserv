#include "client.hpp"
#include "logging.hpp"

static CPPLog::Instance logI = logOut.instance(CPPLog::Level::INFO, "Client");


Client::Client(std::shared_ptr<AsyncIOFD> fd, uint16_t port) : _fd(std::move(fd)), _port(port), _request() {
	logI << "Created client on port " << _port << CPPLog::end;
}

Client::~Client() {
	logI << "Destroyed client on port " << _port << CPPLog::end;
}

AsyncIOFD& Client::fd() {
	return *_fd;
}

uint16_t Client::port() const {
	return _port;
}

httpRequest& Client::request() {
	return _request;
}
