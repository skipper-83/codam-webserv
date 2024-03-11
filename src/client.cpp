#include "client.hpp"

#include "config.hpp"
#include "logging.hpp"
#include "util.hpp"

static CPPLog::Instance clientLogI = logOut.instance(CPPLog::Level::INFO, "client");
static CPPLog::Instance clientLogW = logOut.instance(CPPLog::Level::WARNING, "client");
static CPPLog::Instance clientLogE = logOut.instance(CPPLog::Level::WARNING, "client");

extern MainConfig mainConfig;

void Client::simpleReadCb(AsyncSocketClient& asyncSocketClient) {
    std::string readBuffer;
    try {
        readBuffer = asyncSocketClient.read(DEFAULT_READ_SIZE);
    } catch (const std::exception& e) {
        clientLogE << "SimpleReadCb: " << e.what() << CPPLog::end;
        return;
    }
    clientLogI << "SimpleReadCb: read " << readBuffer.size() << " bytes" << CPPLog::end;
    clientLogI << "SimpleReadCb: " << readBuffer << CPPLog::end;
    this->_tmpFlag = true;
}
void Client::simpleWriteCb(AsyncSocketClient& client) {
    if (_tmpFlag) {
        std::string writeBuffer = "HTTP/1.1 200 OK\r\n\r\njoe\r\n\r\n";
		clientLogI << "Writing " << writeBuffer.size() << " bytes" << CPPLog::end;
        try {
            client.write(writeBuffer);
        } catch (std::exception& e) {
            clientLogW << "error!" << CPPLog::end;
        };
        client.close();
    }
}

void Client::_registerCallbacks() {
    // _socketFd->registerReadReadyCb(std::bind(&Client::simpleReadCb, this, std::placeholders::_1));
    // _socketFd->registerWriteReadyCb(std::bind(&Client::simpleWriteCb, this, std::placeholders::_1));
    _socketFd->registerReadReadyCb(std::bind(&Client::clientReadCb, this, std::placeholders::_1));
    _socketFd->registerWriteReadyCb(std::bind(&Client::clientWriteCb, this, std::placeholders::_1));
}

Client::Client(std::shared_ptr<AsyncSocketClient>& socketFd) : _socketFd(socketFd), _response(&this->_request) {
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


void Client::clientWriteCb(AsyncSocketClient& asyncSocketClient) {
    // If the buffer is empty, return
    if (_localWriteBuffer.empty())
        return;

    size_t bytesWritten = 0;
    std::string writeBuffer;

    // TODO: action if the buffer is too large
    if (_localWriteBuffer.size() + _bytesWrittenCounter > DEFAULT_MAX_WRITE_SIZE) {
        clientLogE << "clientWriteCb: response too large: " << _bytesWrittenCounter + _localWriteBuffer.size() << "fd: " << _socketFd << CPPLog::end;
        _socketFd->close();
		changeState(ClientState::DONE);
        return;
    }

    // If the buffer is too large, write only a part of it
    if (_localWriteBuffer.size() > DEFAULT_WRITE_SIZE) {
        writeBuffer = _localWriteBuffer.substr(0, DEFAULT_WRITE_SIZE);
    } else {
        writeBuffer = _localWriteBuffer;
    }

    try {
		changeState(ClientState::WRITE_RESPONSE);
        bytesWritten = asyncSocketClient.write(writeBuffer);
    } catch (const std::exception& e) {
        clientLogE << "clientWriteCb: " << e.what() << CPPLog::end;
        _state = ClientState::ERROR;
        return;
    }
    _localWriteBuffer.erase(0, bytesWritten);
    _bytesWrittenCounter += bytesWritten;
    clientLogI << "clientWriteCb: wrote " << bytesWritten << " bytes to " << this->_port << CPPLog::end;
    clientLogI << "clientWriteCb: " << _localWriteBuffer.size() << " bytes left to write" << CPPLog::end;
    clientLogI << "clientWriteCb: " << _bytesWrittenCounter << " bytes written in total" << CPPLog::end;
    clientLogI << "clientWriteCb: response body complete? " << this->_response.isBodyComplete() << CPPLog::end;

    // If the response is complete, clear the response and the write counter
    if (_localWriteBuffer.empty() && this->_response.isBodyComplete()) {
        clientLogI << "respnse Connection type" << this->_request.getHeader("Connection") << CPPLog::end;

        if (this->_request.getHeader("Connection") == "close") {
            _socketFd->close();
            clientLogI << "Closing connection by request" << CPPLog::end;
        }
        this->_response.clear();
        this->_request.clear();
        _bytesWrittenCounter = 0;
		changeState(ClientState::READY_FOR_INPUT);
        // _socketFd->close();
    }
}

void Client::clientReadCb(AsyncSocketClient& asyncSocketClient) {
    // Read from the socket and append to the local buffer
    clientLogI << "clientReadCb" << CPPLog::end;
    try {
        _localReadBuffer += asyncSocketClient.read(DEFAULT_READ_SIZE);
    } catch (const std::exception& e) {
        clientLogE << "clientReadCb: " << e.what() << CPPLog::end;
        // _state = ClientState::ERROR;
        return;
    }

    clientLogI << "read: " << _localReadBuffer.size() << " bytes from " << this->_port << CPPLog::end;
    clientLogI << "read: " << _localReadBuffer << CPPLog::end;

    // Try to parse the buffer as http request. If request is incomplete, parse will leave the buffer in place. On error, it will reply with a http
    // error response
    try {
        this->_request.parse(this->_localReadBuffer, this->_port);
		changeState(ClientState::READ_REQUEST);
    } catch (const httpRequest::httpRequestException& e) {
        this->_returnHttpErrorToClient(e.errorNo());
    }

    // If the header is not complete and the buffer is too large, return 413
    if (!this->_request.headerComplete() && _localReadBuffer.size() > DEFAULT_MAX_HEADER_SIZE) {
        this->_returnHttpErrorToClient(413);
    }

    if (this->_request.headerComplete()) {
		changeState(ClientState::READ_BODY);
        clientLogI << "request header complete" << CPPLog::end;
        clientLogI << "request method: " << this->_request.getRequestType() << CPPLog::end;
        clientLogI << "request path: " << this->_request.getAdress() << CPPLog::end;
        clientLogI << "request port: " << this->_request.getPort() << CPPLog::end;
    }

    // Request is complete, handle it
    if (this->_request.bodyComplete()) {
		changeState(ClientState::BUILDING_RESPONSE);
        clientLogI << "request body complete" << CPPLog::end;
        clientLogI << "Body: [" << this->_request.getBody() << "] " << CPPLog::end;
        this->_response.setCode(200);
        this->_response.setFixedSizeBody("Test OK response");
        this->_localWriteBuffer = this->_response.getFixedBodyResponseAsString();
        // this->_localWriteBuffer = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\ntestr\r\n\r\n";
        // this->_request.clear();
        clientLogI << "body complete part done" << CPPLog::end;
    }
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
