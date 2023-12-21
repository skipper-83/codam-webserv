
#include "http_request.hpp"

#include "logging.hpp"

static CPPLog::Instance infoLog = logOut.instance(CPPLog::Level::INFO, "httpRequest parser");
static CPPLog::Instance warningLog = logOut.instance(CPPLog::Level::WARNING, "httpRequest parser");

httpRequest::httpRequest() {}

httpRequest::httpRequest(std::string input) {
    this->parse(input);
}

httpRequest::httpRequest(std::istream &fs) {
    this->parse(fs);
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

bool httpRequest::isComplete(void) const {
    return _requestComplete;
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

void httpRequest::_popLastNewLine(void)
{
	if (_httpRequestBody[_httpRequestBody.size() - 1] == '\n')
	{
		_httpRequestBody.pop_back();
		_bodyLength--;
	}
}

bool httpRequest::addToBody(std::istream &fs) {
    size_t addedLength;
    std::stringstream contents;
	std::string line;

    addedLength = remainingLength(fs);
    infoLog << "ADD TO BODY: " << addedLength;
	if (_requestComplete)
		return true;
    if (_contentLength > 0) {
        if (addedLength + _bodyLength >= _contentLength) {
            try {
                _httpRequestBody += readNumberOfBytesFromFileStream(fs, _contentLength - _bodyLength);
            } catch (const std::exception &e) {
                warningLog << e.what();
                return false;
            }
            _bodyLength = _contentLength;
            _requestComplete = true;
        } else {
            contents << fs.rdbuf();
            _httpRequestBody += contents.str();
            _bodyLength += addedLength;
        }
        infoLog << _httpRequestBody << " " << _bodyLength;
		return true;
    }

	if (_chunkedRequest)
	{
		// chunked request code
		infoLog << "chunked request";
		return true;
	}

	while (std::getline(fs, line))
	{
		if (line.empty())
		{
			fs.clear();
			_popLastNewLine();
			// _httpRequestBody.pop_back(); // remove last newline
			// _bodyLength--; // remove count from last newline
			_requestComplete = true;
			infoLog <<"found double newline: " << _httpRequestBody << " size:" << _bodyLength;
			return true;
		}
		_httpRequestBody += line + "\n";
		_bodyLength += line.size() + 1;
	}
	_popLastNewLine();
	// _httpRequestBody.pop_back(); // remove last newline
	// _bodyLength--; // remove count from last newline
	infoLog <<"found EOF: " << _httpRequestBody << " size:" << _bodyLength;
	fs.clear();
    return true;
}

size_t httpRequest::getBodyLength(void) const {
    return _bodyLength;
}

void httpRequest::parse(std::istream &fs) {
    _getHttpStartLine(fs);
    _getHttpHeaders(fs);
}

void httpRequest::parse(std::string const &input) {
    std::stringstream is(input);
    parse(is);
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
    std::string::size_type key_end, val_start, val_end;

    while (std::getline(fs, line)) {
        if (line.empty())
            break;
        key_end = line.find(':', 0);
        if (key_end == std::string::npos)
            throw std::invalid_argument("Invalid HTTP key/value pair");
        val_start = line.find_first_not_of(' ', key_end + 1);
        if (val_start == std::string::npos)
            val_start = line.size();
        val_end = line.find(':', val_start);
        if (val_end != std::string::npos)
            throw std::invalid_argument("Invalid HTTP key/value pair: two seperators on single line");
        _httpHeaders.insert(std::make_pair(line.substr(0, key_end), line.substr(val_start, val_end)));
    }
    _checkHttpHeaders();
    _setVars();
    return;
}

void httpRequest::_checkHttpHeaders(void) {
    httpRequestT::iterator host_it;

    const static std::regex http_host_header_pattern(
        "(^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\\-]*[a-zA-Z0-9])\\.)*"
        "([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\\-]*[A-Za-z0-9])"
        "(:[0-9]{1,5})?$)|"
        "(^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}"
        "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(:[0-9]{1,5})?$)");
    host_it = _httpHeaders.find("Host");
    if (host_it == _httpHeaders.end() && _httpProtocol == "HTTP/1.1")
        throw std::invalid_argument("No Host specified");
    if (host_it != _httpHeaders.end() && !std::regex_match(host_it->second, http_host_header_pattern))
        throw std::invalid_argument("Invalid Host specified");
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

    if (!(var = getHeader("Content-Length")).empty()) {
        try {
            _contentLength = stoi(var);
        } catch (const std::exception &e) {
            warningLog << e.what() << ": wrong argument for Content-Length";
            _contentLength = 0;
        }
    }

    if (!(var = getHeader("Transfer-Encoding")).empty() && var == "chunked") {
        _chunkedRequest = true;
        _contentLength = 0;
    }
}