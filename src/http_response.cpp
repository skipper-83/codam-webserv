#include "http_response.hpp"

httpResponse::httpResponse() {
	_httpProtocol = DEFAULT_RESPONSE_PROTOCOL;
	setHeader("Content-Type", "text/html; charset=UTF-8");
	setHeader("Server","Jelle en Albert's webserv");


}

void httpResponse::setCode(int code) {
	this->_responseCodeDescription = WebServUtil::codeDescription(code);
	this->_responseCode = code;
}

void httpResponse::setBody(std::string body) {
	this->_httpBody = body;
	this->_bodyLength = this->_httpBody.length();
	deleteHeader("Content-Length");
	setHeader("Content-Length", std::to_string(this->_bodyLength));
}

void httpResponse::setHeader(std::string key, std::string value) {
	this->_httpHeaders.insert(std::make_pair(key, value));
}

std::string httpResponse::getRequestAsString(void)  {
	std::string ret;

	deleteHeader("Date");
	setHeader("Date", WebServUtil::timeStamp());
	ret.append(this->getProtocol()).append(" ").append(std::to_string(this->_responseCode).append(" ").append(this->_responseCodeDescription).append("\n"));
	ret.append(getHeaderListAsString());
	ret.append("\n").append(_httpBody);
    return ret;
}
