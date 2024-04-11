#include "http_request.hpp"
#include "logging.hpp"

static CPPLog::Instance infoLog = logOut.instance(CPPLog::Level::INFO, "httpRequest parser");
static CPPLog::Instance warningLog = logOut.instance(CPPLog::Level::WARNING, "httpRequest parser");

void httpRequest::parse(std::string &input, uint16_t port) {
    if (!this->_headerParseComplete && !this->_hasNewLine(input))  // header is not complete yet
    {
        infoLog << "Header incomplete, leaving buffer in place" << CPPLog::end;
        return;
    }
    std::stringstream is(input);

    if (!this->_headerParseComplete)
        parseHeader(is);
    if (this->_headerParseComplete && this->_server == nullptr) {
        setServer(mainConfig, port);
    }
    if (this->_server)
        _resolvePathAndLocationBlock();
    infoLog << "Checking method if method " << WebServUtil::httpMethodToString( _httpMethod) << " allowed" << CPPLog::end;
    if (this->_server->allowed.methods.find(_httpMethod)->second == false) {
        infoLog << "Method not allowed" << CPPLog::end;
        throw httpRequestException(405, "Method Not Allowed");
    }
    if (this->_headerParseComplete && !this->_bodyComplete) {
        if (WebServUtil::isRequestWithoutBody(this->_httpMethod))
            this->_bodyComplete = true;
        else
            parseBody(is);
    }
    input = is.str().substr(is.tellg());
}