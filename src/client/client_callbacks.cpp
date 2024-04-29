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
	if (_inputFile)
    	_readFromFile();

    if (_cgiMessage) 
		_readFromCgi();
	// {
	// 	int exitCode = _cgiMessage->checkProgramStatus();

	// 	if (exitCode != 0) {
	// 		_cgiMessage = nullptr;
	// 		_returnHttpErrorToClient(500);
	// 		return;
	// 	}
    //     if (_cgiMessage->isBodyComplete()) {
	// 		_response.extractHeaders(_cgiMessage.get());
	// 		if (_response.getHeader("Content-Type").empty())
	// 			_response.setHeader("Content-Type", "text/html; charset=UTF-8");
    //         _response.setCode(200);
    //         _response.setFixedSizeBody(_cgiMessage->getBody());
    //         _clientWriteBuffer = _response.getFixedBodyResponseAsString();
    //         _cgiMessage = nullptr;
    //         _request.clear();
    //     }
    // }
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
        if (this->_request.getHeader("Connection") == "close" ||
		(this->_response.getCode() >= 400 && this->_response.getCode() <= 599)){
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
	if(ClientState::ERROR == _state) // if the client is in error state, do not read
		return;

    // Read from the socket and append to the local buffer
    try {
        _clientReadBuffer += asyncSocketClient.read(DEFAULT_READ_SIZE);
    } catch (const std::exception& e) {
        clientLogE << "_clientReadCb: " << e.what() << CPPLog::end;
        return;
    }

    clientLogI << "read: " << _clientReadBuffer.size() << " bytes from " << this->_port << CPPLog::end;
    if (_clientReadBuffer.size() == 0) // if the buffer is empty, return
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

    if (this->_request.headerComplete() && !this->_request.isSessionSet()) {
        sessionLogI << "Trying to find session" << CPPLog::end;
        sessionLogI << "Cookie Header: " << _request.getHeader("Cookie") << CPPLog::end;
        if (_session == nullptr) {
            sessionLogI << "No session yet" << CPPLog::end;
            if (!_request.getCookie(SESSION_COOKIE_NAME).empty()) {
                try {
                    _session = _sessionList.getSession(_request.getCookie(SESSION_COOKIE_NAME));
                    sessionLogI << "Session found in list: " << _session->getSessionId() << CPPLog::end;
                } catch (const std::exception& e) {
                    sessionLogI << "Session not found in list: " << e.what() << CPPLog::end;
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
        } else {
            sessionLogI << "Session already exists" << CPPLog::end;
		}
        _session->addPathToTrail(_request.getAdress()); // add path to session trail
		_request.setSession(true); // set session flag to true to avoid duplication on longer requests
    }

    // Request is complete, handle it
    if (this->_request.bodyComplete()) {
        changeState(ClientState::BUILDING_RESPONSE);
        
		// If the request is for a CGI script, execute the script
        Cgi const *cgi;
        if ((cgi = _request.getServer()->getCgiFromPath(_request.getPath())) && 
		cgi->allowed.methods.find(_request.getMethod())->second) {
            clientLogI << "CGI request: " << _request.getPath() << "; executor: " << cgi->executor << CPPLog::end;
            try {
                _cgiMessage = std::make_shared<cgiMessage>(cgi->executor, &_request, _addLocalFdToPollArray);
            } catch (const std::exception& e) {
                clientLogE << "_clientReadCb: " << e.what() << CPPLog::end;
                _returnHttpErrorToClient(500);
                return;
            }
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
        switch (this->_request.getMethod()) { // switch block for different request methods
            case WebServUtil::HttpMethod::GET:
            case WebServUtil::HttpMethod::POST:
            case WebServUtil::HttpMethod::HEAD: {
                _openFileAndAddToPollArray(this->_request.getPath());
                _response.setHeader("Content-Type", WebServUtil::getContentTypeFromPath(_request.getPath()));
                break;
            }
            case WebServUtil::HttpMethod::PUT: {
                _returnHttpErrorToClient(999);
                break;
            }
            case WebServUtil::HttpMethod::DELETE: {
				if (int res = std::remove(_request.getPath().c_str()) != 0)
					_returnHttpErrorToClient(500);
				_response.setCode(200);
				_response.setHeader("Content-Type", "text/html; charset=UTF-8");
				_response.setFixedSizeBody("File " + _request.getAdress() + " deleted");
				_clientWriteBuffer = _response.getFixedBodyResponseAsString();
				_request.clear();
                break;
            }
            case WebServUtil::HttpMethod::OPTIONS: {
				_returnHttpErrorToClient(999);
                break;
            }
            default: {
                _returnHttpErrorToClient(405);
                break;
            }
        }
    }
}