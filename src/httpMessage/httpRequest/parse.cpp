#include "httpMessage/http_request.hpp"
#include "logging.hpp"

static CPPLog::Instance infoLog = logOut.instance(CPPLog::Level::INFO, "httpRequest parser");
static CPPLog::Instance warningLog = logOut.instance(CPPLog::Level::WARNING, "httpRequest parser");

void httpRequest::parse(Buffer &input, uint16_t port) {
    if (!this->_headerParseComplete && input.lines() == 0)  // header is not complete yet
    {
        infoLog << "Header incomplete, leaving buffer in place" << CPPLog::end;
        return;
    }
    // std::stringstream is(input);

    if (!this->_headerParseComplete)
        parseHeader(input);
    if (this->_headerParseComplete && this->_server == nullptr) {
        setServer(mainConfig, port);
    }
    if (!_pathSet && this->_server)
	try {
        _resolvePathAndLocationBlock();
	} catch (const httpRequestException &e) {
		throw httpRequestException(e.errorNo(), e.codeDescription());
	}
    // infoLog << "Checking method if method " << WebServUtil::httpMethodToString(_httpMethod) << " allowed" << CPPLog::end;
    if (!_methodCheck && this->_location->allowed.methods.find(_httpMethod)->second == false) {
        infoLog << "Method not allowed" << CPPLog::end;
        throw httpRequestException(405, "Method Not Allowed");
    }
	_methodCheck = true;
    if (this->_headerParseComplete && !this->_bodyComplete) {
        if (WebServUtil::isRequestWithoutBody(this->_httpMethod))
            this->_bodyComplete = true;
        else
            parseBody(input);
    }
    // std::streampos pos = is.tellg();
	// if (pos >= 0 && static_cast<std::size_t>(pos) < is.str().size()) 
    // 	input = is.str().substr(pos);
	// else 
	// 	input = "";
	}
