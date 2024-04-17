#include "http_response.hpp"

#include <sstream>

#include "logging.hpp"

static CPPLog::Instance infoLog = logOut.instance(CPPLog::Level::INFO, "httpResponse");

httpResponse::httpResponse() {
    _httpProtocol = DEFAULT_RESPONSE_PROTOCOL;
    setHeader("Server", DEFAULT_SERVER_NAME);
}

httpResponse::httpResponse(httpRequest* callingRequest) : httpResponse() {
    infoLog << "Constructor with preceding req" << CPPLog::end;
    this->setPrecedingRequest(callingRequest);
}

httpResponse& httpResponse::operator=(const httpResponse& rhs) {
    if (this == &rhs)
        return *this;
    httpMessageAssign(rhs);
    this->_bodyComplete = rhs._bodyComplete;
    this->_responseCode = rhs._responseCode;
    this->_bodyComplete = rhs._bodyComplete;
	this->_chunked = rhs._chunked;
    this->_responseCodeDescription = rhs._responseCodeDescription;
    this->_precedingRequest = rhs._precedingRequest;
    return *this;
}

void httpResponse::setPrecedingRequest(httpRequest* const callingRequest) {
    infoLog << "Setting preceding request" << CPPLog::end;
    _precedingRequest = callingRequest;
}

void httpResponse::setCode(int code) {
    this->_responseCodeDescription = WebServUtil::codeDescription(code);
    this->_responseCode = code;
    if (this->_responseCode >= 400 && this->_responseCode <= 599)
        this->_setErrorBody();
}

void httpResponse::_setErrorBody() {
    std::string error_page;

    infoLog << "Setting up error page for " << this->_responseCode << CPPLog::end;
        error_page.append("<html><head><title>")
            .append(std::to_string(this->_responseCode))
            .append(" " + this->_responseCodeDescription)
            .append("</title></head><body><center><h1>")
            .append(std::to_string(this->_responseCode))
            .append(" " + this->_responseCodeDescription)
            .append("</h1></center><hr><center>")
            .append(DEFAULT_SERVER_NAME)
            .append("</center>");
    // }
    setHeader("Content-Type", "text/html; charset=UTF-8");
    this->setFixedSizeBody(error_page);
    infoLog << "Error page set to: " << error_page << CPPLog::end;
}

void httpResponse::setFixedSizeBody(std::string body) {
    this->_httpBody = body;
    this->_bodyLength = this->_httpBody.length();
    deleteHeader("Content-Length");
    setHeader("Content-Length", std::to_string(this->_bodyLength));
    this->_bodyComplete = true;
}

std::string httpResponse::getStartLine(void) const {
    return this->getProtocol() + " " + std::to_string(this->_responseCode) + " " + this->_responseCodeDescription + "\r\n";
}

std::string httpResponse::getHeadersForChunkedResponse(void) {
    std::string ret;

	_chunked = true;
    deleteHeader("Date");
    setHeader("Date", WebServUtil::timeStamp());
    deleteHeader("Content-Length");
    setHeader("Transfer-Encoding", "chunked");
    ret.append(this->getStartLine());
    ret.append(getHeaderListAsString());
    ret.append("\r\n");
    return ret;
}

std::string httpResponse::transformLineForChunkedResponse(std::string line) {
    std::stringstream ret;

	if (line.empty())
		_bodyComplete = true;
    ret << std::hex << line.size() << "\r\n" << line << "\r\n";
    return (ret.str());
}

std::string httpResponse::getFixedBodyResponseAsString(void) {
    std::string ret;

    deleteHeader("Date");
    setHeader("Date", WebServUtil::timeStamp());
    ret.append(this->getStartLine());
    ret.append(getHeaderListAsString());
    ret.append("\r\n").append(_httpBody).append("\r\n\r\n");
    return ret;
}

bool httpResponse::isBodyComplete(void) const {
    return _bodyComplete;
}

bool httpResponse::isChunked(void) const {
    return _chunked;
}

void httpResponse::clear(void) {
    infoLog << "Clearing response but preserving precedingRequest pointer" << CPPLog::end;
    httpResponse empty;
    const httpRequest* tmpPrecedingRequest = this->_precedingRequest;

    *this = empty;
    this->_precedingRequest = tmpPrecedingRequest;
}
