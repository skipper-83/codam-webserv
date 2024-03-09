#include "client.hpp"
#include "logging.hpp"
#include "config.hpp"
#include "util.hpp"

static CPPLog::Instance clientLogI = logOut.instance(CPPLog::Level::INFO, "client");
static CPPLog::Instance clientLogW = logOut.instance(CPPLog::Level::WARNING, "client");

extern MainConfig mainConfig;

void Client::_registerCallbacks()
{
	_fd->registerReadReadyCb(std::bind(&Client::clientReadCb, this, std::placeholders::_1));
}

Client::Client(std::shared_ptr<AsyncSocketClient> &fd) : _fd(fd), _response(&this->_request) {
	std::cout << "client constructor" << std::endl;
	this->_port = this->_fd->getPort();
	clientLogI << "Created client on port " << _port << CPPLog::end;
	this->_request.setServer(mainConfig, this->_port);
	_registerCallbacks();
}

Client::Client(Client&& other) : _fd(std::move(other._fd)), _port(other._port), _request(std::move(other._request)), _response(&this->_request), _localReadBuffer(std::move(other._localReadBuffer)), _localWriteBuffer(std::move(other._localWriteBuffer)) {

	clientLogI << "Moved client on port " << _port << CPPLog::end;
	_registerCallbacks();
}

Client::~Client() {
	clientLogI << "Destroyed client on port " << _port << CPPLog::end;
}

AsyncIO& Client::fd() {
    return *_fd;
}

uint16_t Client::port() const {
    return _port;
}

void Client::clientReadCb(AsyncSocketClient& client) {
	(void)client;

	// Read from the socket and append to the local buffer
	_localReadBuffer += _fd->read(DEFAULT_READ_SIZE);

	// If the header is not complete and the buffer is too large, return 413
	if (!this->_request.headerComplete() && _localReadBuffer.size() > DEFAULT_MAX_HEADER_SIZE) {
		this->_returnHttpErrorToClient(413);
	}
	clientLogI << "read: " << _localReadBuffer << CPPLog::end;
	
    clientLogI << "received " << _localReadBuffer.size() << " bytes from " << this->_port << CPPLog::end;
    clientLogI << "received: " << _localReadBuffer << CPPLog::end;

	// Try to parse the buffer as http request. If request is incomplete, parse will leave the buffer in place. On error, it will reply with a http error response
    try {
        this->_request.parse(this->_localReadBuffer, this->_port);
    } catch (const httpRequest::httpRequestException& e) {
		this->_returnHttpErrorToClient(e.errorNo());
    }

    if (this->_request.headerComplete()) {
        clientLogI << "request header complete" << CPPLog::end;
        clientLogI << "request method: " << this->_request.getRequestType() << CPPLog::end;
        clientLogI << "request path: " << this->_request.getAdress() << CPPLog::end;
    }

    if (this->_request.bodyComplete()) {
        clientLogI << "request body complete" << CPPLog::end;
        clientLogI << "Body: [" << this->_request.getBody() << "] " << CPPLog::end;
        this->_localWriteBuffer = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\ntestr\r\n\r\n";
        this->_request.clear();
        clientLogI << "body complete part done" << CPPLog::end;
    }
}

void Client::_returnHttpErrorToClient(int code) {
	this->_response.setCode(code);
	this->_localWriteBuffer = this->_response.getResponseAsString();
	this->_localReadBuffer.clear();
	this->_request.clear();
	this->_state = ClientState::WRITE_RESPONSE;
	clientLogW << "HTTP error " << code << ": " << WebServUtil::codeDescription(code);
}
