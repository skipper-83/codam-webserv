#include "client.hpp"

#include "config.hpp"
#include "logging.hpp"
#include "util.hpp"

static CPPLog::Instance clientLogI = logOut.instance(CPPLog::Level::INFO, "client");
static CPPLog::Instance clientLogW = logOut.instance(CPPLog::Level::WARNING, "client");
static CPPLog::Instance clientLogE = logOut.instance(CPPLog::Level::WARNING, "client");

extern MainConfig mainConfig;

void Client::_registerCallbacks() {
    _socketFd->registerReadReadyCb(std::bind(&Client::_clientReadCb, this, std::placeholders::_1));
    _socketFd->registerWriteReadyCb(std::bind(&Client::_clientWriteCb, this, std::placeholders::_1));
}

Client::Client(std::shared_ptr<AsyncSocketClient>& socketFd, std::function<void(std::weak_ptr<AsyncFD>)> addLocalFdToPollArray,
               WebServSessionList& sessionList)
    : _response(&this->_request), _sessionList(sessionList), _socketFd(socketFd), _addLocalFdToPollArray(addLocalFdToPollArray) {
    this->_port = this->_socketFd->getPort();
    // this->_request.setServer(mainConfig, this->_port);
    _registerCallbacks();
}

Client::~Client() {
    clientLogI << "Client destructor called" << CPPLog::end;
}

Client::Client(const Client& rhs) : _sessionList(rhs._sessionList) {
    *this = rhs;
}

Client& Client::operator=(const Client& rhs) {
    if (this == &rhs)
        return *this;
    clientLogI << "Client copy assignment called" << CPPLog::end;
    _socketFd = rhs._socketFd;
    _port = rhs._port;
    _request = rhs._request;
    _response = rhs._response;
    _sessionList = rhs._sessionList;
    _session = rhs._session;
    _state = rhs._state;
    _bytesWrittenCounter = rhs._bytesWrittenCounter;
    _state = rhs._state;
    _clientReadBuffer = rhs._clientReadBuffer;
    _clientWriteBuffer = rhs._clientWriteBuffer;
    _addLocalFdToPollArray = rhs._addLocalFdToPollArray;
    _cgiMessage = rhs._cgiMessage;
    if (_cgiMessage)
        _cgiMessage->setRequest(&_request);
    _inputFile = rhs._inputFile;
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
    clientLogI << "Time changed" << CPPLog::end;
}

void Client::setLastActivityTime() {
    _lastActivityTime = std::chrono::steady_clock::now();
}

std::chrono::time_point<std::chrono::steady_clock> Client::getLastActivityTime() const {
    return _lastActivityTime;
}
