#include <filesystem>

#include "client.hpp"
#include "logging.hpp"

static CPPLog::Instance clientLogI = logOut.instance(CPPLog::Level::INFO, "client");
static CPPLog::Instance clientLogW = logOut.instance(CPPLog::Level::WARNING, "client");
static CPPLog::Instance clientLogE = logOut.instance(CPPLog::Level::WARNING, "client");

void Client::clientWriteCb(AsyncSocketClient& asyncSocketClient) {
    // this bit should probably be done in the FileHandler class
    // ****************************************************** //
    static std::string tempFileBuffer;
    if (_localFd != nullptr && _localFd->hasPendingRead()) {
        clientLogI << "I have a pending read" << CPPLog::end;
        if (_localFd->eof()) {
            clientLogI << "file is at eof" << CPPLog::end;
            _localFd->close();
            _localFd = nullptr;
            _response.setFixedSizeBody(tempFileBuffer);
            _localWriteBuffer = _response.getFixedBodyResponseAsString();
            tempFileBuffer.clear();
        } else {
            tempFileBuffer += _localFd->read(1024);
            clientLogI << "file buffer: " << tempFileBuffer << CPPLog::end;
        }
    }
    // ****************************************************** //

    // If the buffer is empty, return
    if (_localWriteBuffer.empty())
        return;

    size_t bytesWritten = 0;
    std::string writeBuffer;

    // TODO: action if the response is too large
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
    }
}

void Client::clientReadCb(AsyncSocketClient& asyncSocketClient) {
    // Read from the socket and append to the local buffer
    clientLogI << "clientReadCb" << CPPLog::end;
    try {
        _localReadBuffer += asyncSocketClient.read(DEFAULT_READ_SIZE);
    } catch (const std::exception& e) {
        clientLogE << "clientReadCb: " << e.what() << CPPLog::end;
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
        this->_returnHttpErrorToClient(e.errorNo(), e.what());
    }

    // If the header is not complete and the buffer is too large, return 413
    if (!this->_request.headerComplete() && _localReadBuffer.size() > DEFAULT_MAX_HEADER_SIZE) {
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
            _response.setFixedSizeBody(WebServUtil::directoryIndexList(this->_request.getPath()));
            _response.setHeader("Content-Type", "text/html; charset=UTF-8");
			_response.setCode(200);
            _localWriteBuffer = _response.getFixedBodyResponseAsString();
        }
        // ****************************************************** //

        // clientLogI << "request header complete" << CPPLog::end;
        // clientLogI << "request method: " << this->_request.getRequestType() << CPPLog::end;
        // clientLogI << "request path: " << this->_request.getAdress() << CPPLog::end;
        // clientLogI << "request port: " << this->_request.getPort() << CPPLog::end;
    }

    // Request is complete, handle it
    if (this->_request.bodyComplete()) {
        changeState(ClientState::BUILDING_RESPONSE);
        clientLogI << "request body complete" << CPPLog::end;
        clientLogI << "Body: [" << this->_request.getBody() << "] " << CPPLog::end;
        clientLogI << "request address: " << this->_request.getAdress() << CPPLog::end;

        _localFd = AsyncFile::create(_request.getPath(), nullptr);
        if (_localFd) {
            _response.setCode(200);
            _addLocalFdToPollArray(_localFd);
        } else
            _returnHttpErrorToClient(500);  // right now it returns a 500 when the file has incorrect permissions
        clientLogI << "body complete part done" << CPPLog::end;
    }
}