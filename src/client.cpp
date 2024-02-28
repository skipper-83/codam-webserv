#include "client.hpp"
#include "logging.hpp"

static CPPLog::Instance logI = logOut.instance(CPPLog::Level::INFO, "Client");

CPPLog::Instance clienLogI = logOut.instance(CPPLog::Level::INFO, "client");
CPPLog::Instance clientLogW = logOut.instance(CPPLog::Level::WARNING, "client");

Client::Client(std::shared_ptr<AsyncIOFD> fd, uint16_t port) : _fd(std::move(fd)), _port(port), _request() {
	logI << "Created client on port " << _port << CPPLog::end;
}

Client::~Client() {
	logI << "Destroyed client on port " << _port << CPPLog::end;
}

// AsyncIOFD& Client::fd() {
// 	return *_fd;
// }

// uint16_t Client::port() const {
// 	return _port;
// }

void Client::clientReadCb()
{
	if (!this->_fd->hasPendingRead)
                return;
            clienLogI << "received " << this->_fd->readBuffer.size() << " bytes from " << this->_port << CPPLog::end;
            clienLogI << "received: " << this->_fd->readBuffer << CPPLog::end;
			
			
			// this->_fd
            try {
                this->_request.parse(this->_fd->readBuffer, this->_port);
            } catch (const httpRequest::httpRequestException& e) {
                this->_response.setCode(e.errorNo());
                this->_fd->writeBuffer = this->_response.getResponseAsString();
                this->_fd->readBuffer.clear();
                this->_request.clear();
				this->_state = ClientState::WRITE_RESPONSE;
                clientLogW << "HTTP error " << e.errorNo() << ": " << e.codeDescription() << "\n" << e.what();
				// this->_state = ClientState::READY_FOR_INPUT;
            }

            this->_fd->hasPendingRead = false;
            clienLogI << "parsed request from " << this->_port << CPPLog::end;

            if (this->_request.headerComplete()) {
                clienLogI << "request header complete" << CPPLog::end;
                clienLogI << "request method: " << this->_request.getRequestType() << CPPLog::end;
                clienLogI << "request path: " << this->_request.getAdress() << CPPLog::end;
            }

            if (this->_request.bodyComplete()) {
                clienLogI << "request body complete" << CPPLog::end;
                clienLogI << "Body: [" << this->_request.getBody() << "] " << CPPLog::end;
                this->_fd->writeBuffer = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\ntestr\r\n\r\n";
                this->_request.clear();
                clienLogI << "body complete part done" << CPPLog::end;
            }
}

// httpRequest& Client::request() {
// 	return _request;
// }

<<<<<<< Updated upstream
httpRequest& Client::request() {
	return _request;
}
=======
// httpResponse& Client::response() {
//     return _response;
// }
>>>>>>> Stashed changes
