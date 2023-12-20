
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

bool httpRequest::isDone(void) const {
    return _requestDone;
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

std::streampos remainingLength(std::istream &fs) {
    std::streampos curPos, endPos;

    curPos = fs.tellg();
    fs.seekg(0, std::ios::end);
    endPos = fs.tellg();
    fs.seekg(curPos);
    return (endPos - curPos);
}

// void httpRequest::_parseRequestBody(std::istream &fs) {
//     // int limit = 0;
//     std::stringstream body_stream;
//     std::string len;

//     (void)fs;

//     infoLog << "called";
//     if ((len = getHeader("Content-Length")) != "")
//         infoLog << len;
//     body_stream << fs.rdbuf();
//     _httpRequestBody = &body_stream.str()[1];  // skip the newline
// }

void httpRequest::addToBody(std::istream &fs) {
    int addedLength;
    std::stringstream contents;
    
    addedLength = remainingLength(fs);
    infoLog << "ADD TO BODY: " << remainingLength(fs);
    if (_contentLength > 0)
    {
        if (addedLength + _bodyLength >= _contentLength){
            // read up until the _contentLength size;
            // char buffer[]

            // read into buffer the remaining bytes of contentLength
            _requestDone = true;
        }
        else{
            contents << fs.rdbuf();
            _httpRequestBody += contents.str();
            _bodyLength += addedLength;
        }
        infoLog << _httpRequestBody << " " << _bodyLength;
    }
    // case no content size:
    // go through it line by line until line.empty()

    // case chunked
    // got throug it line by line until line is 0\r\n  and then \r\n
    // which would be the same thing?

    // infoLog << fs.rdbuf();

}

void httpRequest::parse(std::istream &fs) {
    // std::stringstream body_stream;
    infoLog << "size before header: " << remainingLength(fs);
    _getHttpStartLine(fs);
    _getHttpHeaders(fs);
    infoLog << "size after header: " << remainingLength(fs);
    // body_stream << fs.rdbuf();
    // _parseRequestBody(fs);
    // _httpRequestBody = &body_stream.str()[1]; //skip the newline
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

    if ((var = getHeader("Content-Length")) != "") {
        try {
            _contentLength = stoi(var);
        } catch (const std::exception &e) {
            warningLog << e.what() << ": wrong argument for Content-Length";
            _contentLength = 0;
        }
    }

    if ((var = getHeader("Transfer-Encoding")) != "" && var == "chunked") {
        _chunkedRequest = true;
        _contentLength = 0;
    }
}