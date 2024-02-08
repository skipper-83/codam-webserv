#include "http_request.hpp"

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
std::string httpRequest::getRequestType(void) const {
    return this->_httpRequestType;
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

/**
 * @brief Sets the server for the request.
 *
 * @param config main config with the configured server blocks
 * @param port port from select()
 */
void httpRequest::setServer(MainConfig &config, uint16_t port) {
    this->_server = config.getServer(port, this->getHeader("Host"));
    if (this->_server == nullptr)
        throw httpRequestException(500, "No server match found");
    this->_clientMaxBodySize = this->_server->clientMaxBodySize.value;
}