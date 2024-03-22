#include "client.hpp"

#include "config.hpp"
#include "logging.hpp"
#include "util.hpp"

static CPPLog::Instance clientLogI = logOut.instance(CPPLog::Level::INFO, "client");
static CPPLog::Instance clientLogW = logOut.instance(CPPLog::Level::WARNING, "client");
static CPPLog::Instance clientLogE = logOut.instance(CPPLog::Level::WARNING, "client");

extern MainConfig mainConfig;

void Client::_registerCallbacks() {
    _socketFd->registerReadReadyCb(std::bind(&Client::clientReadCb, this, std::placeholders::_1));
    _socketFd->registerWriteReadyCb(std::bind(&Client::clientWriteCb, this, std::placeholders::_1));
}

Client::Client(std::shared_ptr<AsyncSocketClient>& socketFd, std::function<void(std::shared_ptr<AsyncFD>)> addLocalFdToPollArray) 
:  _response(&this->_request), _socketFd(socketFd), _addLocalFdToPollArray(addLocalFdToPollArray)
{
	// addLocalFdToPollArray(_socketFd);
    this->_port = this->_socketFd->getPort();
    this->_request.setServer(mainConfig, this->_port);
    _registerCallbacks();
}

Client::Client(Client&& other) :
      _request(std::move(other._request)),
      _response(&this->_request),
      _socketFd(std::move(other._socketFd)),
      _port(other._port),
      _localReadBuffer(std::move(other._localReadBuffer)),
      _localWriteBuffer(std::move(other._localWriteBuffer)),
	  _addLocalFdToPollArray(std::move(other._addLocalFdToPollArray)){
    _registerCallbacks();
}

Client::~Client() {
	if (_localFd != nullptr)
		_localFd->close();
}

Client::Client(const Client& rhs) {
    *this = rhs;
}

Client& Client::operator=(const Client& rhs) {
    if (this == &rhs)
        return *this;
    _socketFd = rhs._socketFd;
    _port = rhs._port;
    _request = rhs._request;
    _response = rhs._response;
    _localReadBuffer = rhs._localReadBuffer;
    _localWriteBuffer = rhs._localWriteBuffer;
    _bytesWrittenCounter = rhs._bytesWrittenCounter;
    _state = rhs._state;
    _clientReadBuffer = rhs._clientReadBuffer;
    _clientWriteBuffer = rhs._clientWriteBuffer;
	_localFd = rhs._localFd;
	_addLocalFdToPollArray = rhs._addLocalFdToPollArray;
    _registerCallbacks();
    return *this;
}

AsyncIO& Client::socketFd() const {
    return *_socketFd;
}

uint16_t Client::port() const {
    return _port;
}

void Client::changeState(ClientState newState) {
	_state = newState;
	setLastActivityTime();
}

void Client::setLastActivityTime() {
    _lastActivityTime = std::chrono::steady_clock::now();
}

std::chrono::time_point<std::chrono::steady_clock> Client::getLastActivityTime() const {
	return _lastActivityTime;
}

// Write an error response to the client
void Client::_returnHttpErrorToClient(int code) {
    this->_response.setCode(code);
    this->_localWriteBuffer = this->_response.getFixedBodyResponseAsString();
    this->_localReadBuffer.clear();
    this->_request.clear();
    this->_state = ClientState::WRITE_RESPONSE;
    clientLogW << "HTTP error " << code << ": " << WebServUtil::codeDescription(code);
}
