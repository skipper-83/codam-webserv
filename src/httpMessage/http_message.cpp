#include "httpMessage/http_message.hpp"

#include <regex>

#include "httpMessage/http_request.hpp"
#include "logging.hpp"

static CPPLog::Instance infoLog = logOut.instance(CPPLog::Level::INFO, "httpRequest body parser");

/**
 * @brief Returns protocol of request (ie HTTP1.1)
 *
 * @return std::string
 */
std::string httpMessage::getProtocol(void) const {
    return _httpProtocol;
}

/**
 * @brief Returns the header with key supplied. If not such key exists returns empty string
 *
 * @param key
 * @return std::string
 */
std::string httpMessage::getHeader(const std::string &key) const {
    if (_httpHeaders.find(key) != _httpHeaders.end())
        return _httpHeaders.find(key)->second;
    return "";
}

/**
 * @brief Returns body of request
 *
 * @return std::string
 */
std::string httpMessage::getBody(void) const {
    return _httpBody;
}

/**
 * @brief Returns the length of the body
 *
 * @return size_t
 */
size_t httpMessage::getBodyLength(void) const {
    return _bodyLength;
}

void httpMessage::deleteHeader(std::string key) {
    while (1) {
        auto it = _httpHeaders.find(key);
        if (it == _httpHeaders.end())
            return;
        _httpHeaders.erase(it);
    }
}

void httpMessage::setHeader(std::string key, std::string value) {
    this->_httpHeaders.insert(std::make_pair(key, value));
}

/**
 * @brief Returns a list of keys with the key supplied
 *
 * @param key
 * @return httpRequest::httpRequestListT
 */
httpMessage::httpRequestListT httpMessage::getHeaderList(std::string const &key) const {
    std::pair<httpRequestT::const_iterator, httpRequestT::const_iterator> range;
    httpRequestListT ret;

    range = _httpHeaders.equal_range(key);
    std::transform(range.first, range.second, std::back_inserter(ret),
                   [](std::pair<std::string, std::string> key_value) { return key_value.second; });
    return ret;
}

httpMessage::httpRequestT httpMessage::getHeaderMap(void) const {
    return _httpHeaders;
}
std::string httpMessage::getHeaderListAsString(void) const {
    std::string ret;
    for (httpRequestT::const_iterator element = _httpHeaders.begin(); element != _httpHeaders.end(); element++)
        ret.append(element->first + ": " + element->second + "\r\n");
    return ret;
}
httpMessage::~httpMessage(void) {}
httpMessage::httpMessage(void) {}

void httpMessage::_httpMessageAssign(const httpMessage &rhs) {
    this->_httpProtocol = rhs._httpProtocol;
    this->_httpHeaders = httpRequestT(rhs._httpHeaders);
    this->_httpBody = rhs._httpBody;
    this->_bodyLength = rhs._bodyLength;
}

bool httpMessage::_getLineWithCRLF(std::istream &is, std::string &line) {
    std::string new_line;
    size_t start_pos = is.tellg();

    // infoLog << "Getting line with CRLF (is type)\n";

    if (!std::getline(is, new_line)) {
        // infoLog << "No line\n";
        line = "";
        return 0;
    }
    infoLog << "Line: [" << line << "]" << CPPLog::end;
    if (!line.empty() && line.back() == '\r') { // no proper line ending
        // infoLog << "Removing \\r\n";
        infoLog << "Inproper line ending: ["<< line.back()<<"]";
        is.seekg(start_pos);
        line ="";
        return 0;
        // line.pop_back();
    }
    line = new_line;
    line.pop_back();
    return 1;
}

std::string httpMessage::_getLineWithCRLF(std::string &input) {
    std::string::size_type pos = input.find('\n');
    std::string line;

    // infoLog << "Getting line with CRLF\n";
    if (pos == std::string::npos || input.empty())
        return "";
    line = input.substr(0, pos);
    if (line[line.size() - 1] == '\r')
        line.pop_back();  // remove \r
    if (pos + 1 < input.size())
        input = input.substr(pos + 1, input.size());  // remove line from input
    return line;
}

std::pair<std::string, std::string> httpMessage::_parseHeaderLine(std::string line) {
    // std::string key, value;
    // std::string::size_type key_end, val_start;

    std::regex pattern("^([^\\s:]+):\\s*(.*)");
    std::smatch match;

    if (std::regex_match(line, match, pattern)) {
        // std::cout << "Header Name: " << match[1] << std::endl;
        // std::cout << "Header Value: " << match[2] << std::endl;
    } else {
        throw std::runtime_error("Invalid header line");
    }
    return std::make_pair(match[1], match[2]);
}
