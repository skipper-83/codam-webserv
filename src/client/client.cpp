#include "client.hpp"

#include "config.hpp"
#include "logging.hpp"
#include "util.hpp"

static CPPLog::Instance clientLogI = logOut.instance(CPPLog::Level::INFO, "client");
static CPPLog::Instance clientLogW = logOut.instance(CPPLog::Level::WARNING, "client");
static CPPLog::Instance clientLogE = logOut.instance(CPPLog::Level::WARNING, "client");

extern MainConfig mainConfig;

// void Client::simpleReadCb(AsyncSocketClient& asyncSocketClient) {
//     std::string readBuffer;
//     try {
//         readBuffer = asyncSocketClient.read(DEFAULT_READ_SIZE);
//     } catch (const std::exception& e) {
//         clientLogE << "SimpleReadCb: " << e.what() << CPPLog::end;
//         return;
//     }
//     clientLogI << "SimpleReadCb: read " << readBuffer.size() << " bytes" << CPPLog::end;
//     clientLogI << "SimpleReadCb: " << readBuffer << CPPLog::end;
//     this->_tmpFlag = true;
// }
// void Client::simpleWriteCb(AsyncSocketClient& client) {
//     if (_tmpFlag) {
//         std::string writeBuffer = "HTTP/1.1 200 OK\r\n\r\njoe\r\n\r\n";
// 		clientLogI << "Writing " << writeBuffer.size() << " bytes" << CPPLog::end;
//         try {
//             client.write(writeBuffer);
//         } catch (std::exception& e) {
//             clientLogW << "error!" << CPPLog::end;
//         };
//         client.close();
//     }
// }

void Client::_registerCallbacks() {
    // _socketFd->registerReadReadyCb(std::bind(&Client::simpleReadCb, this, std::placeholders::_1));
    // _socketFd->registerWriteReadyCb(std::bind(&Client::simpleWriteCb, this, std::placeholders::_1));
    _socketFd->registerReadReadyCb(std::bind(&Client::clientReadCb, this, std::placeholders::_1));
    _socketFd->registerWriteReadyCb(std::bind(&Client::clientWriteCb, this, std::placeholders::_1));
}

Client::Client(std::shared_ptr<AsyncSocketClient>& socketFd, std::function<void(std::shared_ptr<AsyncFD>)> addLocalFdToPollArray) 
: _socketFd(socketFd), _response(&this->_request), _addLocalFdToPollArray(addLocalFdToPollArray)
{
	// addLocalFdToPollArray(_socketFd);
    std::cout << "client constructor" << std::endl;
    this->_port = this->_socketFd->getPort();
    clientLogI << "Created client on port " << _port << CPPLog::end;
    this->_request.setServer(mainConfig, this->_port);
    _registerCallbacks();
}

Client::Client(Client&& other)
    : _socketFd(std::move(other._socketFd)),
      _port(other._port),
      _request(std::move(other._request)),
      _response(&this->_request),
      _localReadBuffer(std::move(other._localReadBuffer)),
      _localWriteBuffer(std::move(other._localWriteBuffer)) {
    clientLogI << "Moved client on port " << _port << CPPLog::end;
    _registerCallbacks();
}

Client::~Client() {
	if (_localFd != nullptr)
		_localFd->close();
    clientLogI << "Destroyed client on port " << _port << CPPLog::end;
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
