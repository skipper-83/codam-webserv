#include "client.hpp"
#include "logging.hpp"

static CPPLog::Instance clientLogI = logOut.instance(CPPLog::Level::INFO, "client");
static CPPLog::Instance clientLogW = logOut.instance(CPPLog::Level::WARNING, "client");
static CPPLog::Instance clientLogE = logOut.instance(CPPLog::Level::WARNING, "client");

void Client::_buildResponse() {
    // If the request is for a CGI script, execute the script
    Cgi const* cgi;
    if ((cgi = _request.getServer()->getCgiFromPath(_request.getPath())) && cgi->allowed.methods.find(_request.getMethod())->second) {
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
		try
		{
        	_response.setFixedSizeBody(WebServUtil::directoryIndexList(this->_request.getPath(), _request.getAdress()));
		}
		catch(const std::exception& e)
		{
			_returnHttpErrorToClient(500);
		}
		
        _response.setHeader("Content-Type", "text/html; charset=UTF-8");
        _response.setCode(200);
        _clientWriteBuffer = _response.getFixedBodyResponseAsString();
        _request.clear();
        return;
    }

    // If the request is for a file, return the file with the appropriate method
    switch (this->_request.getMethod()) {
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
            if (std::remove(_request.getPath().c_str()) != 0)
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