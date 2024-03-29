#include <filesystem>

#include "client.hpp"
#include "logging.hpp"

static CPPLog::Instance clientLogI = logOut.instance(CPPLog::Level::INFO, "client");
static CPPLog::Instance clientLogW = logOut.instance(CPPLog::Level::WARNING, "client");
static CPPLog::Instance clientLogE = logOut.instance(CPPLog::Level::WARNING, "client");

void Client::clientWriteCb(AsyncSocketClient& asyncSocketClient) {
    // _inputFile->read();
    if (_inputFile) {
        clientLogI << "I have an inputfile" << CPPLog::end;

		if (_inputFile->bad()) {
			clientLogE << "file is bad" << CPPLog::end;
			if (_inputFile->badCode() == EACCES)
			{
				_inputFile = nullptr;
				_returnHttpErrorToClient(403);
			}
			else
				_returnHttpErrorToClient(500);
			return;
		}

		if (_inputFile->readBufferFull()) 
		{
			clientLogI << "file buffer full" << CPPLog::end;
			_response.setCode(200);
			if (!_response.isChunked())
				_clientWriteBuffer = _response.getHeadersForChunkedResponse();
			_clientWriteBuffer += _response.transformLineForChunkedResponse(_inputFile->read());
			clientLogI << "file buffer: " << _clientWriteBuffer << "size:" << _clientWriteBuffer.size() << CPPLog::end;
		}

        if (_inputFile->eof()) {
            clientLogI << "file is at eof" << CPPLog::end;
            std::string tempFileBuffer = _inputFile->read();
			_inputFile = nullptr;
            clientLogI << "file buffer: " << tempFileBuffer << "size:" << tempFileBuffer.size() << CPPLog::end;
			if (!_response.isChunked())
			{
				_response.setCode(200);
				_response.setFixedSizeBody(tempFileBuffer);
            	_clientWriteBuffer = _response.getFixedBodyResponseAsString();
			}
			else
			{
				_clientWriteBuffer += _response.transformLineForChunkedResponse(tempFileBuffer);
				_clientWriteBuffer += _response.transformLineForChunkedResponse("");
			}
		}
	}

    // If the buffer is empty, return
    if (_clientWriteBuffer.empty())
        return;

    size_t bytesWrittenInCycle = 0;
    std::string writeCycleBuffer;

    // TODO: action if the response is too large
    if (_clientWriteBuffer.size() + _bytesWrittenCounter > DEFAULT_MAX_WRITE_SIZE) {
        clientLogE << "clientWriteCb: response too large: " << _bytesWrittenCounter + _clientWriteBuffer.size() << "fd: " << _socketFd << CPPLog::end;
        _clientWriteBuffer.clear();
		if (_response.isChunked())
			_clientWriteBuffer = _response.transformLineForChunkedResponse("");
		
        _returnHttpErrorToClient(413);
        return;
    }

    // If the buffer is too large, write only a part of it
    if (_clientWriteBuffer.size() > DEFAULT_WRITE_SIZE) {
        writeCycleBuffer = _clientWriteBuffer.substr(0, DEFAULT_WRITE_SIZE);
    } else {
        writeCycleBuffer = _clientWriteBuffer;
    }

    try {
        changeState(ClientState::WRITE_RESPONSE);
        bytesWrittenInCycle = asyncSocketClient.write(writeCycleBuffer);
    } catch (const std::exception& e) {
        clientLogE << "clientWriteCb: " << e.what() << CPPLog::end;
        changeState(ClientState::ERROR);
        return;
    }
    _clientWriteBuffer.erase(0, bytesWrittenInCycle);
    _bytesWrittenCounter += bytesWrittenInCycle;
    clientLogI << "clientWriteCb: wrote " << bytesWrittenInCycle << " bytes to " << this->_port << CPPLog::end;
    clientLogI << "clientWriteCb: " << _clientWriteBuffer.size() << " bytes left to write" << CPPLog::end;
    clientLogI << "clientWriteCb: " << _bytesWrittenCounter << " bytes written in total" << CPPLog::end;
    clientLogI << "clientWriteCb: response body complete? " << this->_response.isBodyComplete() << CPPLog::end;

    // If the response is complete, clear the response and the write counter
    if (_clientWriteBuffer.empty() && this->_response.isBodyComplete()) {
        clientLogI << "respnse Connection type: " << this->_request.getHeader("Connection") << CPPLog::end;

        if (this->_request.getHeader("Connection") == "close") {
            _socketFd->close();
            clientLogI << "Closing connection by request" << CPPLog::end;
        }
        this->_response.clear();
        this->_request.clear();
        _bytesWrittenCounter = 0;
        clientLogI << "clientWriteCb: client ready for input" << CPPLog::end;
        changeState(ClientState::READY_FOR_INPUT);
    }
}

void Client::clientReadCb(AsyncSocketClient& asyncSocketClient) {
    // Read from the socket and append to the local buffer
    clientLogI << "clientReadCb" << CPPLog::end;
    try {
        _clientReadBuffer += asyncSocketClient.read(DEFAULT_READ_SIZE);
    } catch (const std::exception& e) {
        clientLogE << "clientReadCb: " << e.what() << CPPLog::end;
        return;
    }

    clientLogI << "read: " << _clientReadBuffer.size() << " bytes from " << this->_port << CPPLog::end;
    clientLogI << "read: " << _clientReadBuffer << CPPLog::end;
    if (_clientReadBuffer.size() == 0)
        return;

    // Try to parse the buffer as http request. If request is incomplete, parse will leave the buffer in place. On error, it will reply with a http
    // error response
    try {
        this->_request.parse(this->_clientReadBuffer, this->_port);
        changeState(ClientState::READ_REQUEST);
    } catch (const httpRequest::httpRequestException& e) {
        this->_returnHttpErrorToClient(e.errorNo(), e.what());
    }

    // If the header is not complete and the buffer is too large, return 413
    if (!this->_request.headerComplete() && _clientReadBuffer.size() > DEFAULT_MAX_HEADER_SIZE) {
        this->_returnHttpErrorToClient(413);
    }

    if (this->_request.headerComplete()) {
        changeState(ClientState::READ_BODY);

        // this should be done in the FileHandler class
        // ****************************************************** //
        if (!std::filesystem::exists(this->_request.getPath())) {
            _returnHttpErrorToClient(404);
        }
        if (std::filesystem::is_directory(this->_request.getPath())) {
            _response.setFixedSizeBody(WebServUtil::directoryIndexList(this->_request.getPath(), _request.getAdress()));

            _response.setHeader("Content-Type", "text/html; charset=UTF-8");
            _response.setCode(200);
            _clientWriteBuffer = _response.getFixedBodyResponseAsString();
            _request.clear();
        }
        // ****************************************************** //
    }

    // Request is complete, handle it
    if (this->_request.bodyComplete()) {
        changeState(ClientState::BUILDING_RESPONSE);
        clientLogI << "request body complete" << CPPLog::end;
        clientLogI << "Body: [" << this->_request.getBody() << "] " << CPPLog::end;
        clientLogI << "request address: " << this->_request.getAdress() << CPPLog::end;

        clientLogI << "making Infilehandler" << CPPLog::end;
        _inputFile = std::make_shared<InFileHandler>(this->_request.getPath(), 1024);
		_addLocalFdToPollArray(_inputFile->operator std::shared_ptr<AsyncFD>());
		_response.setHeader("Content-Type", WebServUtil::getContentTypeFromPath(_request.getPath()));
        clientLogI << "body complete part done" << CPPLog::end;
    }
}