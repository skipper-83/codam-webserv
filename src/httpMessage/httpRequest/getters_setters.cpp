#include "httpMessage/http_request.hpp"
#include "logging.hpp"

static CPPLog::Instance infoLog = logOut.instance(CPPLog::Level::INFO, "httpRequest parser");
/**
 * @brief Returns address of request
 *
 * @return std::string
 */
std::string httpRequest::getAdress(void) const {
    return this->_httpAdress;
}

/**
 * @brief Returns type of request (ie GET, POST)
 *
 * @return std::string
 */
WebServUtil::HttpMethod httpRequest::getMethod(void) const {
    return this->_httpMethod;
}

std::string httpRequest::getErrorPage(int errorCode) const {
    if (this->_server == nullptr)
        return std::string();
    return (this->_server->getErrorPage(errorCode));
}

/**
 * @brief Set to true if the parsing of the body of the request is complete
 *
 * @return true
 * @return false
 */
bool httpRequest::bodyComplete(void) const {
    return _bodyComplete;
}

/**
 * @brief Set to true if the parsing of the headers of the request is complete
 *
 * @return true
 * @return false
 */
bool httpRequest::headerComplete(void) const {
    return _headerParseComplete;
}

bool httpRequest::returnAutoIndex(void) const {
    return _returnAutoIndex;
}

/**
 * @brief Sets the server for the request.
 *
 * @param config main config with the configured server blocks
 * @param port port from select()
 */
void httpRequest::setServer(MainConfig &config, uint16_t port) {
    infoLog << "setServer: " << port << " host:" << this->getHeader("Host") << CPPLog::end;
    this->_server = config.getServer(port, this->getHeader("Host"));
    if (this->_server == nullptr)
        throw httpRequestException(500, "No server match found");
    // this->_clientMaxBodySize = this->_server->clientMaxBodySize.value;
}

const ServerConfig *httpRequest::getServer(void) const {
    return _server;
}

std::string httpRequest::getPath(void) const {
    return _path;
}

std::map<std::string, std::string> httpRequest::getCookies(void) const {
    return (_cookies);
}

std::string httpRequest::getCookie(std::string key) const {
    if (_cookies.find(key) == _cookies.end())
        return std::string();
    return _cookies.at(key);
}

const Location *httpRequest::getLocation(void) const {
    return _location;
}
