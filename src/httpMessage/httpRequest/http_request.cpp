#include "httpMessage/http_request.hpp"

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
    _httpMessageAssign(rhs);
    this->_httpMethod = rhs._httpMethod;
    this->_httpAdress = rhs._httpAdress;
    this->_cookies = rhs._cookies;
    this->_headerParseComplete = rhs._headerParseComplete;
    this->_bodyComplete = rhs._bodyComplete;
    this->_contentLength = rhs._contentLength;
    this->_chunkedRequest = rhs._chunkedRequest;
    this->_contentSizeSet = rhs._contentSizeSet;
    this->_server = rhs._server;
    this->_port = rhs._port;
    this->_clientMaxBodySize = rhs._clientMaxBodySize;
    this->_returnAutoIndex = rhs._returnAutoIndex;
    this->_pathSet = rhs._pathSet;
    this->_sessionSet = rhs._sessionSet;
    this->_path = rhs._path;
    this->_queryString = rhs._queryString;
    this->_nextChunkSize = rhs._nextChunkSize;
    this->_chunkSizeKnown = rhs._chunkSizeKnown;
    this->_firstNewLineFound = rhs._firstNewLineFound;
    return *this;
}

void httpRequest::clear(Buffer &buffer) {
    infoLog << "Clearing request" << CPPLog::end;
    httpRequest empty;
    *this = empty;
    infoLog << "Request cleared, buffer now: " << buffer.read(buffer.size()) << CPPLog::end;
}

uint16_t httpRequest::getPort(void) const {
    return _port;
}

std::ostream &operator<<(std::ostream &os, httpRequest const &t) {
    os << "Protocol:" << t.getProtocol() << "\n"
       << "Type: " << WebServUtil::httpMethodToString(t.getMethod()) << "\n"
       << "Adress: " << t.getAdress() << "\n"
       << t.getHeaderListAsString() << "\n"
       << t.getBody() << "\n";
    return os;
}
