
#include "http_request.hpp"

#include <map>

#include "logging.hpp"

static CPPLog::Instance infoLog = logOut.instance(CPPLog::Level::INFO, "httpRequest parser");
static CPPLog::Instance warningLog = logOut.instance(CPPLog::Level::WARNING, "httpRequest parser");

/**
 * @brief Construct a new http Request::http Request object
 *
 */
httpRequest::httpRequest() {}

/**
 * @brief Construct a new http Request::http Request object
 *
 * @param input
 */
httpRequest::httpRequest(std::string input) {
    this->parse(input);
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
    this->_httpRequestType = rhs._httpRequestType;
    this->_httpAdress = rhs._httpAdress;
    this->_httpProtocol = rhs._httpProtocol;
    this->_httpHeaders = httpRequestT(rhs._httpHeaders);
    this->_httpBody = rhs._httpBody;
    this->_headerParseComplete = rhs._headerParseComplete;
    this->_contentLength = rhs._contentLength;
    this->_chunkedRequest = rhs._chunkedRequest;
    this->_contentSizeSet = rhs._contentSizeSet;
    this->_server = rhs._server;
    this->_port = rhs._port;
    this->_clientMaxBodySize = rhs._clientMaxBodySize;
    return *this;
}

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

std::string httpRequest::getErrorPage(int errorCode) const{
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
 * @brief Returns the remaining amount of bytes from the current pos of the read pointer
 * 		untill the end. Resets the readpointer to its current position after this operation.
 *
 * @param fs
 * @return std::streampos
 */
std::streampos remainingLength(std::istream &fs) {
    std::streampos curPos, endPos;

    curPos = fs.tellg();
    fs.seekg(0, std::ios::end);
    endPos = fs.tellg();
    fs.seekg(curPos);
    return (endPos - curPos);
}

/**
 * @brief Helper function. Reads the amount of bytes set in [amountOfBytes] and returns them as a string.
 * Throws expection if the filestream is shorter than the amount of bytes asked for.
 *
 * @param fs
 * @param amountOfBytes
 * @return std::string
 */
static std::string readNumberOfBytesFromFileStream(std::istream &fs, size_t amountOfBytes) {
    std::vector<char> buffer(amountOfBytes);

    if (!fs.read(buffer.data(), amountOfBytes)) {
        throw(std::runtime_error("Failed to read chunk from http request"));
    }
    std::string ret(buffer.begin(), buffer.end());
    return (ret);
}

/**
 * @brief Reads from filestream until it reaches the size set in the header
 *
 * @param fs
 */
void httpRequest::_addToFixedContentSize(std::istream &fs) {
    std::stringstream contents;
    std::streampos addedLength;

    addedLength = remainingLength(fs);
    if ((size_t)addedLength + this->_bodyLength >= this->_contentLength) {
        try {
            _httpBody += readNumberOfBytesFromFileStream(fs, _contentLength - _bodyLength);
        } catch (const std::exception &e) {
            throw httpRequestException(400, "Error reading http request: " + std::string(e.what()));
        }
        this->_bodyLength = _contentLength;
        _bodyComplete = true;
    } else {
        contents << fs.rdbuf();
        this->_httpBody += contents.str();
        this->_bodyLength += addedLength;
    }
    infoLog << this->_httpBody << " " << this->_bodyLength;
    return;
}

/**
 * @brief Add to body from a chunked filestream. Expects the amount of bytes as a number on one line, then that amount of bytes to read on the next.
 * Keeps reading until it encountes 0\n\n.
 *
 * @param fs
 */
void httpRequest::_addChunkedContent(std::istream &fs) {
    std::string line;
    std::streampos nextChunkSize;

    while (std::getline(fs, line)) {
        try {
            nextChunkSize = stoi(line);
        } catch (std::exception &e) {
            throw httpRequestException(400, "Incorrect chunk size in chunked http request: " + std::string(e.what()));
        }
        if (remainingLength(fs) < nextChunkSize) {
            throw httpRequestException(400, "Chunk size smaller than announced size in chunked http request");
        }
        if (this->_bodyLength + nextChunkSize > this->_clientMaxBodySize)
            throw httpRequestException(413, "Request body larger than max body size");
        if (!nextChunkSize) {
            std::getline(fs, line);
            if (line.empty()) {
                this->_bodyComplete = true;
                return;
            }
            throw httpRequestException(400, "Terminating line of chunked http request non-empty");
        }
        try {
            this->_httpBody += readNumberOfBytesFromFileStream(fs, nextChunkSize);
        } catch (const std::exception &e) {
            throw httpRequestException(400, "Error reading chunked http request: " + std::string(e.what()));
        }
        this->_bodyLength += (size_t)nextChunkSize;
        std::getline(fs, line);  // skip terminating newline
    }
    fs.clear();
    return;
}

/**
 * @brief Reads from filestream until an empty line is encountered.
 *
 * @param fs
 */
void httpRequest::_addUntilNewline(std::istream &fs) {
    std::string line;
    while (std::getline(fs, line)) {
        if (line.empty()) {
            fs.clear();
            this->_bodyComplete = true;
            infoLog << "found double newline: " << this->_httpBody << " size:" << this->_bodyLength;
            if (_httpBody[_httpBody.size() - 1] != '\n') {
                this->_httpBody += '\n';
                ++this->_bodyLength;
            }
            return;
        }
        if (line.size() + this->_bodyLength + static_cast<int>(fs.eof()) > this->_clientMaxBodySize)
            throw httpRequestException(413, "Request body larger than max body size");
        this->_httpBody += line;
        this->_bodyLength += line.size();
        infoLog << "eof bit: " << fs.eof();
        if (!fs.eof()) {
            this->_httpBody += '\n';
            this->_bodyLength++;
        }
    }
    infoLog << "found EOF: " << this->_httpBody << " size:" << this->_bodyLength;
    fs.clear();
    return;
}

/**
 * @brief Adds from the filestream to the body of the request until the stop condition (depending on header) is encountered
 *
 * @param fs
 */
void httpRequest::addToBody(std::istream &fs) {
    std::streampos nextChunkSize;
    std::stringstream contents;
    std::string line;

    infoLog << "ADD TO BODY: " << remainingLength(fs);
    if (this->_bodyComplete)
        return;
    if (this->_chunkedRequest)
        return _addChunkedContent(fs);
    if (this->_contentSizeSet)
        return _addToFixedContentSize(fs);
    _addUntilNewline(fs);
}

/**
 * @brief Sets the server for the request.
 *
 * @param config main config with the configured server blocks
 * @param port port from select()
 */
void httpRequest::setServer(MainConfig &config, int port) {
    if (!this->_headerParseComplete)
        return;  // might handle this differently, this is here now to avoid segfaults.
    // if (this->_port > -1 && _port != port)
    //     throw httpRequestException(400, "Transmission port does not match port in Host header");
    this->_server = config.getServer(port, this->getHeader("Host"));
    if (this->_server == nullptr)
        throw std::runtime_error("No server match found");
    this->_clientMaxBodySize = this->_server->clientMaxBodySize.value;
}

void httpRequest::parseHeader(std::istream &fs) {
    _getHttpStartLine(fs);
    _getHttpHeaders(fs);
    this->_headerParseComplete = true;
    // addToBody(fs);
}

void httpRequest::parse(std::string &input) {
    if (!this->_headerParseComplete && input.find("\n\n") == std::string::npos)
        return;
    std::stringstream is(input);

    if (!this->_headerParseComplete) {
        try {
            parseHeader(is);
        } catch (const httpRequestException &e) {
            // reply with bad request response?
            warningLog << "HTTP error " << e.errorNo() << ": " << e.codeDescription() << "\n" << e.what();
        }
    }
    try {
        addToBody(is);
    } catch (const httpRequestException &e) {
        // reply with bad request response?
        warningLog << "HTTP error " << e.errorNo() << ": " << e.codeDescription() << "\n" << e.what();
    }
    std::cerr << "is length now: [" << remainingLength(is) << "]\n";
    input = is.str().substr(is.tellg());
    // if (remainingLength(is))
    // 	input = is.str();
    // else
    // 	input = "";
}

void httpRequest::_getHttpStartLine(std::istream &fs) {
    std::string line;

    std::getline(fs, line);
    std::string::size_type request_type_pos, address_pos;

    static const std::regex http_startline_pattern(
        "^(GET|POST|PUT|DELETE|HEAD|OPTIONS|PATCH) "
        "((https?://[\\w-]+(\\.[\\w-]+)*/?)?/[^ ]*?(\\?[^ ]+?)?) "
        "HTTP/\\d\\.\\d$");
    if (!std::regex_match(line, http_startline_pattern))
        throw httpRequestException(400, "Invalid HTTP start line");
    request_type_pos = line.find(' ');
    address_pos = line.find(' ', request_type_pos + 1);
    this->_httpRequestType = line.substr(0, request_type_pos);
    this->_httpAdress = line.substr(request_type_pos + 1, address_pos - request_type_pos - 1);
    this->_httpProtocol = line.substr(address_pos + 1, request_type_pos - address_pos - 1);
    return;
}

void httpRequest::_getHttpHeaders(std::istream &fs) {
    std::string line;
    std::string::size_type key_end, val_start;

    while (std::getline(fs, line)) {
        if (line.empty())
            break;
        key_end = line.find(':', 0);
        if (key_end == std::string::npos)
            throw httpRequestException(400, "Invalid HTTP key/value pair");
        val_start = line.find_first_not_of(' ', key_end + 1);
        if (val_start == std::string::npos)
            val_start = line.size();
        this->_httpHeaders.insert(std::make_pair(line.substr(0, key_end), line.substr(val_start, line.size() - 1)));
    }
    _checkHttpHeaders();
    _setVars();
    return;
}

void httpRequest::_checkHttpHeaders(void) {
    httpRequestT::iterator host_it;

    static const std::regex http_host_header_pattern(
        "(^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\\-]*[a-zA-Z0-9])\\.)*"
        "([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\\-]*[A-Za-z0-9])"
        "(:[0-9]{1,5})?$)|"
        "(^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}"
        "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(:[0-9]{1,5})?$)");
    host_it = _httpHeaders.find("Host");
    if (host_it == this->_httpHeaders.end() && this->_httpProtocol == "HTTP/1.1")
        throw httpRequestException(400, "No host specified");
    if (host_it != this->_httpHeaders.end() && !std::regex_match(host_it->second, http_host_header_pattern))
        throw httpRequestException(400, "Invalid host specified");
    return;
}

std::ostream &operator<<(std::ostream &os, httpRequest const &t) {
    os << "Protocol:" << t.getProtocol() << "\n"
       << "Type: " << t.getRequestType() << "\n"
       << "Adress: " << t.getAdress() << "\n"
       << t.getHeaderListAsString() << "\n"
       << t.getBody() << "\n";
    return os;
}

/**
 * @brief Set private vars according to headers
 *  - content_length
 *  - chunked (default: false)
 *  - strip port number from host line
 *
 */
void httpRequest::_setVars(void) {
    std::string var;
    std::string::size_type pos;
    httpRequestT::iterator it;

    if (!(var = getHeader("Content-Length")).empty()) {
        try {
            this->_contentLength = stoi(var);
        } catch (const std::exception &e) {
            warningLog << e.what() << ": wrong argument for Content-Length";
            this->_contentLength = 0;
        }
        this->_contentSizeSet = true;
    }

    if (!(var = getHeader("Transfer-Encoding")).empty() && var == "chunked") {
        this->_chunkedRequest = true;
        this->_contentLength = 0;
    }

    if ((it = this->_httpHeaders.find("Host")) != this->_httpHeaders.end() && (pos = it->second.find(':')) != std::string::npos) {
        var = it->second;
        this->_httpHeaders.erase(it);
        this->_httpHeaders.insert({"Host", var.substr(0, pos)});
        infoLog << var.substr(pos + 1, var.size() - pos - 1);
        this->_port = stoi(var.substr(pos + 1, var.size() - pos - 1));
    }
}