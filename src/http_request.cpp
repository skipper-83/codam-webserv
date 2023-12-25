
#include <map>

#include "http_request.hpp"
#include "logging.hpp"

static CPPLog::Instance infoLog = logOut.instance(CPPLog::Level::INFO, "httpRequest parser");
static CPPLog::Instance warningLog = logOut.instance(CPPLog::Level::WARNING, "httpRequest parser");

httpRequest::httpRequest() {}

httpRequest::httpRequest(std::string input) {
    this->parse(input);
}

httpRequest::httpRequest(std::istream &fs) {
    this->parseHeader(fs);
}

httpRequest::httpRequest(const httpRequest &src) {
    *this = src;
}

httpRequest::~httpRequest() {}

httpRequest &httpRequest::operator=(const httpRequest &rhs) {
    if (this == &rhs)
        return *this;
    this->_httpRequestType = rhs._httpRequestType;
    this->_httpAdress = rhs._httpAdress;
    this->_httpProtocol = rhs._httpProtocol;
    this->_httpHeaders = httpRequestT(rhs._httpHeaders);
    this->_httpRequestBody = rhs._httpRequestBody;
    return *this;
}

std::string httpRequest::getAdress(void) const {
    return _httpAdress;
}

std::string httpRequest::getRequestType(void) const {
    return _httpRequestType;
}

std::string httpRequest::getProtocol(void) const {
    return _httpProtocol;
}

std::string httpRequest::getHeader(const std::string &key) const {
    if (_httpHeaders.find(key) != _httpHeaders.end())
        return _httpHeaders.find(key)->second;
    return "";
}

std::string httpRequest::getBody(void) const {
    return _httpRequestBody;
}

bool httpRequest::bodyComplete(void) const {
    return _bodyComplete;
}

bool httpRequest::headerComplete(void) const {
    return _headerParseComplete;
}

httpRequest::httpRequestListT httpRequest::getHeaderList(std::string const &key) const {
    std::pair<httpRequestT::const_iterator, httpRequestT::const_iterator> range;
    httpRequestListT ret;

    range = _httpHeaders.equal_range(key);
    std::transform(range.first, range.second, std::back_inserter(ret),
                   [](std::pair<std::string, std::string> key_value) { return key_value.second; });
    return ret;
}

void httpRequest::printHeaders(std::ostream &os) const {
    for (httpRequestT::const_iterator element = _httpHeaders.begin(); element != _httpHeaders.end(); element++)
        os << element->first << ": " << element->second << "\n";
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

static std::string readNumberOfBytesFromFileStream(std::istream &fs, size_t amountOfBytes) {
    std::vector<char> buffer(amountOfBytes);

    if (!fs.read(buffer.data(), amountOfBytes)) {
        throw(std::runtime_error("Failed to read chunk from http request"));
    }
    std::string ret(buffer.begin(), buffer.end());
    return (ret);
}

void httpRequest::addToBody(std::istream &fs) {
    std::streampos addedLength, nextChunkSize;
    std::stringstream contents;
    std::string line;

    addedLength = remainingLength(fs);
    infoLog << "ADD TO BODY: " << addedLength;
    if (_bodyComplete)
        return ;
    if (_contentSizetSet) {
        if ((size_t)addedLength + _bodyLength >= _contentLength) {
            try {
                _httpRequestBody += readNumberOfBytesFromFileStream(fs, _contentLength - _bodyLength);
            } catch (const std::exception &e) {
				throw httpRequestException(400, "Error reading http request: " + std::string(e.what()));
            }
            _bodyLength = _contentLength;
            _bodyComplete = true;
        } else {
            contents << fs.rdbuf();
            _httpRequestBody += contents.str();
            _bodyLength += addedLength;
        }
        infoLog << _httpRequestBody << " " << _bodyLength;
        return ;
    }

    if (_chunkedRequest) {
        while (std::getline(fs, line)) {
            try {
                nextChunkSize = stoi(line);
            } catch (std::exception &e) {
				throw httpRequestException(400, "Incorrect chunk size in chunked http request: "  + std::string(e.what()));
            }
            if (remainingLength(fs) < nextChunkSize) {
				throw httpRequestException(400, "Chunk size smaller than announced size in chunked http request");
            }
			if (_bodyLength + nextChunkSize > _clientMaxBodySize)
				throw httpRequestException(413, "Request body larger than max body size");
            if (!nextChunkSize)
			{
				std::getline(fs, line);
				if (line.empty())
				{
					_bodyComplete = true;
            		return ;
				}
				throw httpRequestException(400, "Terminating line of chunked http request non-empty");
			}
            try {
                _httpRequestBody += readNumberOfBytesFromFileStream(fs, nextChunkSize);
            } catch (const std::exception &e) {
				throw httpRequestException(400, "Error reading chunked http request: " + std::string(e.what()));
            }
			_bodyLength += (size_t)nextChunkSize;
			std::getline(fs, line); // skip terminating newline
        }
		fs.clear();
        return ;
    }

    while (std::getline(fs, line)) {
        if (line.empty()) {
            fs.clear();
            _bodyComplete = true;
            infoLog << "found double newline: " << _httpRequestBody << " size:" << _bodyLength;
			if (_httpRequestBody[_httpRequestBody.size() - 1] != '\n')
			{
				_httpRequestBody += 'n';
				++_bodyLength;
			}
            return ;
        }
		if (line.size() + _bodyLength + static_cast<int>(fs.eof()) > _clientMaxBodySize)
			throw httpRequestException(413, "Request body larger than max body size");
        _httpRequestBody += line;
        _bodyLength += line.size();
		infoLog << "eof bit: " << fs.eof();
		if (!fs.eof())
		{
			_httpRequestBody += '\n';
			_bodyLength++;
		}
    }
    infoLog << "found EOF: " << _httpRequestBody << " size:" << _bodyLength;
    fs.clear();
    return ;
}

size_t httpRequest::getBodyLength(void) const {
    return _bodyLength;
}

void httpRequest::setServer(MainConfig &config, int port) {
	
	
	if (!_headerParseComplete)
		return ; // might handle this differently, this is here now to avoid segfaults.
	if (_port > -1 && _port != port)
		throw httpRequestException(400, "Transmission port does not match port in Host header");
	this->_server = config.getServer(port, this->getHeader("Host"));
	if (this->_server == nullptr)
		throw std::runtime_error("No server match found");
	_clientMaxBodySize = _server->clientMaxBodySize.value;
	// return ;
	// if (config.getServerFromPortAndName(port, this->getHeader("Host")) != nullptr)
}

void httpRequest::parseHeader(std::istream &fs) {
    _getHttpStartLine(fs);
    _getHttpHeaders(fs);
	_headerParseComplete = true;
    // addToBody(fs);
}

void httpRequest::parse(std::string const &input) {
    std::stringstream is(input);
    parseHeader(is);
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
        throw std::invalid_argument("Invalid HTTP start line");
    request_type_pos = line.find(' ');
    address_pos = line.find(' ', request_type_pos + 1);
    _httpRequestType = line.substr(0, request_type_pos);
    _httpAdress = line.substr(request_type_pos + 1, address_pos - request_type_pos - 1);
    _httpProtocol = line.substr(address_pos + 1, request_type_pos - address_pos - 1);
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
            throw std::invalid_argument("Invalid HTTP key/value pair");
        val_start = line.find_first_not_of(' ', key_end + 1);
        if (val_start == std::string::npos)
            val_start = line.size();
        _httpHeaders.insert(std::make_pair(line.substr(0, key_end), line.substr(val_start, line.size() - 1)));
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
    if (host_it == _httpHeaders.end() && _httpProtocol == "HTTP/1.1")
        throw httpRequestException(400, "No host specified");
    if (host_it != _httpHeaders.end() && !std::regex_match(host_it->second, http_host_header_pattern))
        throw httpRequestException(400, "Invalid host specified");
    return;
}

std::ostream &operator<<(std::ostream &os, httpRequest const &t) {
    os << "Protocol:" << t.getProtocol() << "\n"
       << "Type: " << t.getRequestType() << "\n"
       << "Adress: " << t.getAdress() << "\n";
    t.printHeaders(os);
    os << "\n" << t.getBody() << "\n";
    return os;
}

void httpRequest::_setVars(void) {
    std::string var;
	std::string::size_type pos;
	httpRequestT::iterator it;

    if (!(var = getHeader("Content-Length")).empty()) {
        try {
            _contentLength = stoi(var);
        } catch (const std::exception &e) {
            warningLog << e.what() << ": wrong argument for Content-Length";
            _contentLength = 0;
			// we could throw a 400 here
        }
		_contentSizetSet = true;
    }

    if (!(var = getHeader("Transfer-Encoding")).empty() && var == "chunked") {
        _chunkedRequest = true;
        _contentLength = 0;
    }

	if ((it = _httpHeaders.find("Host")) != _httpHeaders.end() && (pos = it->second.find(':')) != std::string::npos)
	{
		var = it->second;
		_httpHeaders.erase(it);
		_httpHeaders.insert({"Host", var.substr(0, pos)});
		infoLog << var.substr(pos + 1, var.size() - pos - 1);
		_port = stoi(var.substr(pos + 1, var.size() - pos - 1));
	}	
}