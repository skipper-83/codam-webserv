
#include "http_request.hpp"
// #include <map>
#include "logging.hpp"

static CPPLog::Instance infoLog = logOut.instance(CPPLog::Level::INFO, "httpRequest parser");
// static CPPLog::Instance warningLog = logOut.instance(CPPLog::Level::WARNING, "httpRequest parser");

/**
 * @brief Construct a new http Request::http Request object
 *
 */
httpRequest::httpRequest() {infoLog << "Creating empty request" << this << CPPLog::end;}

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
    this->_httpRequestType = rhs._httpRequestType;
    this->_httpAdress = rhs._httpAdress;
    this->_httpProtocol = rhs._httpProtocol;
    this->_httpHeaders = httpRequestT(rhs._httpHeaders);
    this->_httpBody = rhs._httpBody;
    this->_bodyLength = rhs._bodyLength;
    this->_headerParseComplete = rhs._headerParseComplete;
    this->_bodyComplete = rhs._bodyComplete;
    this->_contentLength = rhs._contentLength;
    this->_chunkedRequest = rhs._chunkedRequest;
    this->_contentSizeSet = rhs._contentSizeSet;
    this->_server = rhs._server;
    this->_port = rhs._port;
    this->_clientMaxBodySize = rhs._clientMaxBodySize;
    return *this;
}

void httpRequest::clear(void) {
    infoLog << "Clearing request" << CPPLog::end;
    httpRequest empty;
    *this = empty;
}

std::ostream &operator<<(std::ostream &os, httpRequest const &t) {
    os << "Protocol:" << t.getProtocol() << "\n"
       << "Type: " << t.getRequestType() << "\n"
       << "Adress: " << t.getAdress() << "\n"
       << t.getHeaderListAsString() << "\n"
       << t.getBody() << "\n";
    return os;
}

// /**
//  * @brief Construct a new http Request::http Request object
//  *
//  * @param input
//  */
// httpRequest::httpRequest(std::string input) {
//     this->parse(input);
// }
