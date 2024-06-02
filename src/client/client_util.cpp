#include "client.hpp"
#include "config.hpp"
#include "logging.hpp"

static CPPLog::Instance clientLogI = logOut.instance(CPPLog::Level::INFO, "client");
static CPPLog::Instance clientLogW = logOut.instance(CPPLog::Level::WARNING, "client");

void Client::_openFileAndAddToPollArray(std::string path) {
    _inputFile = std::make_shared<InFileHandler>(path, DEFAULT_LOCAL_FILE_READBUFFER);
    _addLocalFdToPollArray(_inputFile->operator std::shared_ptr<AsyncFD>());
}

// Write an error response to the client
void Client::_returnHttpErrorToClient(int code, std::string message) {
    std::string error_path, error_page;

    clientLogI << "Returning error to client: " << code << " " << message << CPPLog::end;
    // if custom error page is set, use it
    this->_clientWriteBuffer.clear();
    this->_clientReadBuffer.clear();
    this->_response.deleteHeader("Content-Type");
    this->_response.setCode(code);
    // clientLogI << _response.getFixedBodyResponseAsString() << CPPLog::end;
    if (code == 301)  // if the """error""" is a redirect, set the location header
        _response.setHeader("Location", message);
    if (_request.getServer() && !(error_page = _request.getServer()->getErrorPage(code)).empty()) {
        clientLogI << "Error page found: " << error_path << CPPLog::end;
        // root path for server is to be prependended to the error path
        for (auto& it : _request.getServer()->locations) {
            if (it.ref == "/") {
                error_path = it.root + error_page;
                break;
            }
        }
        clientLogI << "Error page found: " << error_path << CPPLog::end;
        if (error_path.empty())
            error_path = DEFAULT_ROOT + error_page;
        if (std::filesystem::exists(error_path)) {
            clientLogI << "Opening error page: " << error_path << CPPLog::end;
            _openFileAndAddToPollArray(error_path);
        } else
            this->_clientWriteBuffer = this->_response.getFixedBodyResponseAsString();
    } else {
        this->_clientWriteBuffer = this->_response.getFixedBodyResponseAsString();
    }
    this->_request.clear();
    changeState(ClientState::ERROR);
    //
    clientLogI << "Buffer from error now: " << this->_clientWriteBuffer.read(_clientWriteBuffer.size()) << CPPLog::end;
    clientLogW << "HTTP error " << code << ": " << WebServUtil::codeDescription(code);
}