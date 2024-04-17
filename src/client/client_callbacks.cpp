#include <filesystem>
// #include "httpMessage/cgi_message.hpp"
#include "client.hpp"
#include "logging.hpp"

static CPPLog::Instance clientLogI = logOut.instance(CPPLog::Level::INFO, "client");
static CPPLog::Instance clientLogW = logOut.instance(CPPLog::Level::WARNING, "client");
static CPPLog::Instance clientLogE = logOut.instance(CPPLog::Level::WARNING, "client");
static CPPLog::Instance sessionLogI = logOut.instance(CPPLog::Level::DEBUG, "WebServSession");

void Client::_clientWriteCb(AsyncSocketClient& asyncSocketClient) {
    // read from file (if available)
    _readFromFile();

    if (_cgiMessage) {
        if (_cgiMessage->isBodyComplete()) {
            clientLogI << "_clientWriteCb: CGI response complete" << CPPLog::end;
            clientLogI << "_clientWriteCb: CGI response: " << _cgiMessage->getBody() << CPPLog::end;
            _response.setHeader("Content-Type", "text/html; charset=UTF-8");
            _response.setCode(200);
            _response.setFixedSizeBody(_cgiMessage->getBody());
            _clientWriteBuffer = _response.getFixedBodyResponseAsString();
            _cgiMessage = nullptr;
            _request.clear();
            return;
        }
    }
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
        if (this->_request.getHeader("Connection") == "close") {
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

void Client::_cgiReadCb(AsyncProgram& cgi) {
    if (cgi.eof()) {
        // clientLogI << "_cgiReadCb: EOF" << CPPLog::end;
        return;
    }

    std::string cgiReadCycleBuffer;

    try {
        cgiReadCycleBuffer = cgi.read(DEFAULT_READ_SIZE);
    } catch (const std::exception& e) {
        clientLogE << "_cgiReadCb: " << e.what() << CPPLog::end;
        _returnHttpErrorToClient(500);
        return;
    }

    if (cgiReadCycleBuffer.empty())
        return;

    clientLogI << "_cgiReadCb: " << cgiReadCycleBuffer << CPPLog::end;
}

void Client::_cgiWriteCb(AsyncProgram& cgi) {
    if (_requestBodyForCgi.empty() == false)
        clientLogI << "_cgiWriteCb: " << cgi.write(_requestBodyForCgi) << CPPLog::end;
}

void Client::_clientReadCb(AsyncSocketClient& asyncSocketClient) {
    // Read from the socket and append to the local buffer
    clientLogI << "_clientReadCb" << CPPLog::end;
    try {
        _clientReadBuffer += asyncSocketClient.read(DEFAULT_READ_SIZE);
    } catch (const std::exception& e) {
        clientLogE << "_clientReadCb: " << e.what() << CPPLog::end;
        return;
    }

    clientLogI << "read: " << _clientReadBuffer.size() << " bytes from " << this->_port << CPPLog::end;
    clientLogI << "read: " << _clientReadBuffer << CPPLog::end;
    if (_clientReadBuffer.size() == 0)
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

    if (this->_request.headerComplete()) {
        sessionLogI << "Trying to find session" << CPPLog::end;
        sessionLogI << "Cookie Header: " << _request.getHeader("Cookie") << CPPLog::end;
        if (_session == nullptr) {
            sessionLogI << "No session yet" << CPPLog::end;
            if (!_request.getCookie(SESSION_COOKIE_NAME).empty()) {
                try {
                    _session = _sessionList.getSession(_request.getCookie(SESSION_COOKIE_NAME));
                    sessionLogI << "Session found in list: " << _session->getSessionId() << CPPLog::end;
                } catch (const std::exception& e) {
                    sessionLogI << "Session not foun in list: " << e.what() << CPPLog::end;
                    _session = _sessionList.createSession();
                    sessionLogI << "Session added: " << CPPLog::end;
                    sessionLogI << _session->getSessionId() << CPPLog::end;
                    _session->setSessionIdToResponse(_response);
                    sessionLogI << "Session added: " << _session->getSessionId() << CPPLog::end;
                }
            } else {
                sessionLogI << "No session cookie" << CPPLog::end;
                _session = _sessionList.createSession();
                _session->setSessionIdToResponse(_response);
                sessionLogI << "Session added: " << _session->getSessionId() << CPPLog::end;
            }
            changeState(ClientState::READ_BODY);
        } else
            sessionLogI << "Session already exists" << CPPLog::end;
        _session->addPathToTrail(_request.getAdress());
    }
    // Request is complete, handle it
    if (this->_request.bodyComplete()) {
        changeState(ClientState::BUILDING_RESPONSE);
        std::string cgiExecutor;
        // If the request is for a CGI script, execute the script
        if ((cgiExecutor = _request.getServer()->getCgiExecutorFromPath(_request.getPath())).empty() == false) {
            clientLogI << "CGI request: " << _request.getPath() << "; executor: " << cgiExecutor << CPPLog::end;
            // _response.setFixedSizeBody("CGI not implemented yet, this script would be executed by: " + cgiExecutor);
            // _response.setHeader("Content-Type", "text/html; charset=UTF-8");
            // _response.setCode(200);
            // _clientWriteBuffer = _response.getFixedBodyResponseAsString();
            // _cgi = AsyncProgram::create(cgiExecutor, _request.getPath(), {}, std::bind(&Client::_cgiReadCb, this, std::placeholders::_1),
            // std::bind(&Client::_cgiWriteCb, this, std::placeholders::_1)); _cgi->addToPollArray(_addLocalFdToPollArray);
            try {
                _cgiMessage = std::make_shared<cgiMessage>(cgiExecutor, &_request, _addLocalFdToPollArray);
            } catch (const std::exception& e) {
                clientLogE << "_clientReadCb: " << e.what() << CPPLog::end;
                _returnHttpErrorToClient(500);
                return;
            }
            // _requestBodyForCgi = _request.getBody();
            // _request.clear();
            return;
        }

        // If the request is for a directory, return the directory index
        if (_request.returnAutoIndex()) {
            _response.setFixedSizeBody(WebServUtil::directoryIndexList(this->_request.getPath(), _request.getAdress()));
            _response.setHeader("Content-Type", "text/html; charset=UTF-8");
            _response.setCode(200);
            _clientWriteBuffer = _response.getFixedBodyResponseAsString();
            _request.clear();
            return;
        }

        // ************************************************ //
        // HERE SWITCH BLOCK FOR STATIC FILE SERVING		//
        // WITH DIFFERENT LOGIC FOR EACH METHOD				//
        // ************************************************ //
        switch (this->_request.getMethod()) {
            case WebServUtil::HttpMethod::GET:
            case WebServUtil::HttpMethod::POST:
            case WebServUtil::HttpMethod::HEAD: {
                // If the request is for a file, open the file and add it to the poll array
                _openFileAndAddToPollArray(this->_request.getPath());
                _response.setHeader("Content-Type", WebServUtil::getContentTypeFromPath(_request.getPath()));
                break;
            }
            case WebServUtil::HttpMethod::PUT: {
                // PUT logic here
                break;
            }
            case WebServUtil::HttpMethod::DELETE: {
                // DELETE logic here
                break;
            }
            case WebServUtil::HttpMethod::OPTIONS: {
                // OPTIONS logic here
                break;
            }
            default: {
                _returnHttpErrorToClient(405);
                break;
            }
        }

        // If the request is for a file, open the file and add it to the poll array
        // _openFileAndAddToPollArray(this->_request.getPath());
        // _response.setHeader("Content-Type", WebServUtil::getContentTypeFromPath(_request.getPath()));
    }
}