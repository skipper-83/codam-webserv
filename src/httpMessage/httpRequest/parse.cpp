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
    if (this->_headerParseComplete && this->_server == nullptr)
        setServer(mainConfig, port);
    if (this->_headerParseComplete && !this->_bodyComplete) {
        if (WebServUtil::isRequestWithoutBody(this->_httpRequestType))
            this->_bodyComplete = true;
        else
            parseBody(is);
    }
    input = is.str().substr(is.tellg());
}