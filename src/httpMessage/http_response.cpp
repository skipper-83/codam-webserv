#include "http_response.hpp"

#include "logging.hpp"

static CPPLog::Instance infoLog = logOut.instance(CPPLog::Level::INFO, "httpResponse");

httpResponse::httpResponse() {
    _httpProtocol = DEFAULT_RESPONSE_PROTOCOL;
    setHeader("Content-Type", "text/html; charset=UTF-8");
    setHeader("Server", DEFAULT_SERVER_NAME);
}

httpResponse::httpResponse(httpRequest* callingRequest) {
    httpResponse();
	infoLog << "Constructor with preceding req" << CPPLog::end;
    this->setPrecedingRequest(callingRequest);
}

void httpResponse::setPrecedingRequest(httpRequest* const callingRequest) {
	infoLog << "Setting preceding request" << CPPLog::end;
    _precedingRequest = callingRequest;
}

void httpResponse::setCode(int code) {
    this->_responseCodeDescription = WebServUtil::codeDescription(code);
    this->_responseCode = code;
    if (this->_responseCode >= 400 && this->_responseCode <= 599)
        this->setErrorBody();
}

void httpResponse::setErrorBody() {
    std::string error_page;

    infoLog << "Setting up error page for " << this->_responseCode << CPPLog::end;
    if (this->_precedingRequest != nullptr && this->_precedingRequest->getServer() != nullptr)
        error_page = this->_precedingRequest->getServer()->getErrorPage(this->_responseCode);
    if (error_page.empty()) {
        error_page.append("<html><head><title>")
            .append(std::to_string(this->_responseCode))
            .append(" " + this->_responseCodeDescription)
            .append("</title></head><body><center><h1>")
            .append(std::to_string(this->_responseCode))
            .append(" " + this->_responseCodeDescription)
            .append("</h1></center><hr><center>")
			.append(DEFAULT_SERVER_NAME)
			.append("</center>");
    }
    this->setBody(error_page);
	infoLog << "Error page set to: " << error_page << CPPLog::end;
}

void httpResponse::setBody(std::string body) {
    this->_httpBody = body;
    this->_bodyLength = this->_httpBody.length();
    deleteHeader("Content-Length");
    setHeader("Content-Length", std::to_string(this->_bodyLength));
}

std::string httpResponse::getResponseAsString(void) {
    std::string ret;

    deleteHeader("Date");
    setHeader("Date", WebServUtil::timeStamp());
    ret.append(this->getProtocol())
        .append(" ")
        .append(std::to_string(this->_responseCode).append(" ").append(this->_responseCodeDescription).append("\r\n"));
    ret.append(getHeaderListAsString());
    ret.append("\r\n").append(_httpBody).append("\r\n\r\n");
    return ret;
}
