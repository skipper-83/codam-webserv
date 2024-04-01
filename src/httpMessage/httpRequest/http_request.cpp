
#include "http_request.hpp"
// #include <map>
#include "logging.hpp"

static CPPLog::Instance infoLog = logOut.instance(CPPLog::Level::INFO, "httpRequest parser");
// static CPPLog::Instance warningLog = logOut.instance(CPPLog::Level::WARNING, "httpRequest parser");

/**
 * @brief Construct a new http Request::http Request object
 *
 */
httpRequest::httpRequest() {
    infoLog << "Creating empty request" << this << CPPLog::end;
}

/**
 * @brief Construct a new http Request::http Request object from an input stream
 *
 * @param fs
 */
httpRequest::httpRequest(std::istream &fs) {
    this->parseHeader(fs);
}

/**
 * @brief CCopy constructor for httpRequest
 *
 * @param src
 */
httpRequest::httpRequest(const httpRequest &src) {
    *this = src;
}

/**
 * @brief Destroy the http Request::http Request object
 *
 */
httpRequest::~httpRequest() {}

/**
 * @brief Copy assignment operator.
 *
 * @param rhs
 * @return httpRequest&
 */
httpRequest &httpRequest::operator=(const httpRequest &rhs) {
    if (this == &rhs)
        return *this;

    // static_cast<httpMessage &>(*this) = rhs;
    httpMessageAssign(rhs);
    this->_httpMethod = rhs._httpMethod;
    this->_httpAdress = rhs._httpAdress;
    this->_headerParseComplete = rhs._headerParseComplete;
    this->_bodyComplete = rhs._bodyComplete;
    this->_contentLength = rhs._contentLength;
    this->_chunkedRequest = rhs._chunkedRequest;
    this->_contentSizeSet = rhs._contentSizeSet;
    this->_server = rhs._server;
    this->_port = rhs._port;
    this->_clientMaxBodySize = rhs._clientMaxBodySize;
	this->_returnAutoIndex = rhs._returnAutoIndex;
    this->_path = rhs._path;
    return *this;
}

void httpRequest::clear(void) {
    infoLog << "Clearing request" << CPPLog::end;
    httpRequest empty;
    *this = empty;
}

uint16_t httpRequest::getPort(void) const {
    return _port;
}

std::ostream &operator<<(std::ostream &os, httpRequest const &t) {
    os << "Protocol:" << t.getProtocol() << "\n"
       << "Type: " << WebServUtil::httpMethodToString(t.getRequestType()) << "\n"
       << "Adress: " << t.getAdress() << "\n"
       << t.getHeaderListAsString() << "\n"
       << t.getBody() << "\n";
    return os;
}
