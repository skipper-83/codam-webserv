#include "client.hpp"
#include "logging.hpp"


static CPPLog::Instance clientLogI = logOut.instance(CPPLog::Level::INFO, "client");
static CPPLog::Instance clientLogW = logOut.instance(CPPLog::Level::WARNING, "client");
static CPPLog::Instance clientLogE = logOut.instance(CPPLog::Level::WARNING, "client");

void Client::clientWriteCb(AsyncSocketClient& asyncSocketClient) {
    // If the buffer is empty, return
    if (_localWriteBuffer.empty())
        return;

    size_t bytesWritten = 0;
    std::string writeBuffer;
	if (_localFd !=nullptr )
	{
		clientLogI << "I have a pending read" << CPPLog::end;
		clientLogI << _localFd->read(1024) << CPPLog::end;
	}
	else
		clientLogI << "I don't have a pending read" << CPPLog::end;

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
		clientLogI << "request address: " << this->_request.getAdress() << CPPLog::end;
		if (_request.getAdress() == "/conf.conf")
			_localFd = _localFd->create("/Users/albertvanandel/Documents/CODAM/webserv/build" + _request.getAdress(), [this](AsyncFD& _localFd) {
				// (void)_localFd;
				AsyncFile& file = static_cast<AsyncFile&>(_localFd);
				clientLogI << "I'm in the callback" << CPPLog::end;
				// clientLogI << file.read(1024) << CPPLog::end;
				this->_localWriteBuffer = file.read(1024);
				if (file.eof())
				{
					clientLogI << "file is at eof" << CPPLog::end;
					file.close();
					// _localWriteBuffer = _response.getFixedBodyResponseAsString();
					// clientLogI << "adding to pollarray" << CPPLog::end;
					// _addLocalFdToPollArray(_localFd);
					// clientLogI << "body complete part done" << CPPLog::end;
				}
			});
		// _localWriteBuffer = "HTTP/1.1 200 OK\r\nContent-Length: 3\r\n\r\njoe\r\n\r\n";
		_response.setFixedSizeBody("joe");
		_localWriteBuffer = _response.getFixedBodyResponseAsString();
		clientLogI << "adding to pollarray" << CPPLog::end;
		if(_localFd)
			_addLocalFdToPollArray(_localFd);
        clientLogI << "body complete part done" << CPPLog::end;
    }
}