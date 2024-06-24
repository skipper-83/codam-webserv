#include "client.hpp"
#include "logging.hpp"
static CPPLog::Instance clientLogI = logOut.instance(CPPLog::Level::INFO, "client");

void Client::_readFromCgi() {
    int exitCode = _cgiMessage->checkProgramStatus();

    if (exitCode != 0) {
        _cgiMessage = nullptr;
        _returnHttpErrorToClient(500);
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
}