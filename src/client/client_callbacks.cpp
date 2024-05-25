#include <filesystem>
// #include "httpMessage/cgi_message.hpp"
#include "client.hpp"
#include "logging.hpp"

static CPPLog::Instance clientLogI = logOut.instance(CPPLog::Level::INFO, "client");
static CPPLog::Instance clientLogW = logOut.instance(CPPLog::Level::WARNING, "client");
static CPPLog::Instance clientLogE = logOut.instance(CPPLog::Level::WARNING, "client");
// static CPPLog::Instance sessionLogI = logOut.instance(CPPLog::Level::DEBUG, "WebServSession");

void Client::_clientWriteCb(AsyncSocketClient& asyncSocketClient) {
    // read from file (if available)
    if (_inputFile)
        _readFromFile();

    if (_cgiMessage)
        _readFromCgi();

    // If the buffer is empty, return
    if (_clientWriteBuffer.empty())
        return;

    size_t bytesWrittenInCycle = 0;
    std::string writeCycleBuffer;

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
        clientLogE << "_clientWriteCb: " << e.what() << CPPLog::end;
        changeState(ClientState::ERROR);
        return;
    }
    _clientWriteBuffer.erase(0, bytesWrittenInCycle);
    _bytesWrittenCounter += bytesWrittenInCycle;
    clientLogI << "_clientWriteCb: wrote " << bytesWrittenInCycle << " bytes to " << this->_port << CPPLog::end;
    clientLogI << "_clientWriteCb: " << _clientWriteBuffer.size() << " bytes left to write" << CPPLog::end;
    clientLogI << "_clientWriteCb: " << _bytesWrittenCounter << " bytes written in total" << CPPLog::end;
    clientLogI << "_clientWriteCb: response body complete? " << this->_response.isBodyComplete() << CPPLog::end;

    // If the response is complete, clear the response and the write counter
    if (_clientWriteBuffer.empty() && this->_response.isBodyComplete()) {
        clientLogI << "respnse Connection type: " << this->_request.getHeader("Connection") << CPPLog::end;
        if (this->_request.getHeader("Connection") == "close" || (this->_response.getCode() >= 400 && this->_response.getCode() <= 599)) {
            _socketFd->close();
            clientLogI << "Closing connection by request" << CPPLog::end;
        }
        this->_response.clear();
        this->_request.clear();
        _inputFile = nullptr;
        _bytesWrittenCounter = 0;
        clientLogI << "_clientWriteCb: client ready for input" << CPPLog::end;
        changeState(ClientState::READY_FOR_INPUT);
    }
}
void Client::_clientReadCb(AsyncSocketClient& asyncSocketClient) {
    // clientLogI << "_clientReadCb" << CPPLog::end;
    if (ClientState::ERROR == _state)  // if the client is in error state, do not read
        return;

    // Read from the socket and append to the local buffer
    try {
        _clientReadBuffer += asyncSocketClient.read(DEFAULT_READ_SIZE);
    } catch (const std::exception& e) {
        clientLogE << "_clientReadCb: " << e.what() << CPPLog::end;
        return;
    }

    clientLogI << "read: " << _clientReadBuffer.size() << " bytes from " << this->_port << CPPLog::end;
    clientLogI << "buffer contents: " << _clientReadBuffer << CPPLog::end;
    if (_clientReadBuffer.size() == 0)  // if the buffer is empty, return
        return;

    // Try to parse the buffer as http request. If request is incomplete, parse will leave the buffer in place. On error, it will reply with a http
    // error response
    try {
        changeState(ClientState::READ_REQUEST);
        this->_request.parse(this->_clientReadBuffer, this->_port);
    } catch (const httpRequest::httpRequestException& e) {
        this->_returnHttpErrorToClient(e.errorNo(), e.what());
    }

    // If the header is not complete and the buffer is too large, return 413
    if (!this->_request.headerComplete() && _clientReadBuffer.size() > DEFAULT_MAX_HEADER_SIZE) {
        this->_returnHttpErrorToClient(413);
    }

    // If the header is complete and the session is not set, handle the session
    if (this->_request.headerComplete() && !this->_request.isSessionSet())
        _handleSession();

    // Request is complete, handle it
    if (this->_request.bodyComplete()) {
        changeState(ClientState::BUILDING_RESPONSE);
        _buildResponse();
    }
}