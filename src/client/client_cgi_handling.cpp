#include "client.hpp"
#include "logging.hpp"
static CPPLog::Instance clientLogI = logOut.instance(CPPLog::Level::INFO, "client");

void Client::_readFromCgi() {
    int exitCode = _cgiMessage->checkProgramStatus();

    if (std::chrono::steady_clock::now() - _cgiMessage->getStartTime() > CGI_TIMEOUT) {
        _cgiMessage = nullptr;
        _returnHttpErrorToClient(408);
        return;
    }
    if (exitCode != 0) {
        if (_cgiMessage->isTooLarge()) {
            _cgiMessage = nullptr;
            _returnHttpErrorToClient(413);
            return;
        }
        std::string cgiErrorMsg = _cgiMessage->getBuffer();
        _response.extractHeaders(_cgiMessage.get());
        _cgiMessage = nullptr;
        clientLogI << "CGI error: " << cgiErrorMsg << CPPLog::end;
        std::string cgiErorCode = _response.getHeader("Status");
        if (!cgiErorCode.empty())
            _returnHttpErrorToClient(std::stoi(cgiErorCode), cgiErrorMsg);
        else
            _returnHttpErrorToClient(500, cgiErrorMsg);
        // _returnHttpErrorToClient(500, cgiErrorMsg);
        return;
    }
    changeState(ClientState::CGI_WRITE);
    if (_cgiMessage->isBodyComplete()) {
        changeState(ClientState::CGI_READ);
        _response.extractHeaders(_cgiMessage.get());
        if (_response.getHeader("Content-Type").empty())
            _response.setHeader("Content-Type", "text/html; charset=UTF-8");
        _response.setCode(200);
        _response.setFixedSizeBody(_cgiMessage->getBody());
        _clientWriteBuffer = _response.getFixedBodyResponseAsString();
        _cgiMessage = nullptr;
        _request.clear(this->_clientReadBuffer);
    }
    // else if (_cgiMessage->getBody().size() > CGI_MAX_BODY_SIZE) {
    //     _cgiMessage = nullptr;
    //     _returnHttpErrorToClient(413);
    // }
}