
#include "http_request.hpp"
#include "http_message.hpp"



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
	while (1)
	{
		auto it = _httpHeaders.find(key);
		if (it == _httpHeaders.end())
			return ;
		_httpHeaders.erase(it);
	}
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

std::string httpMessage::getHeaderListAsString(void) const {
	std::string ret;

    for (httpRequestT::const_iterator element = _httpHeaders.begin(); element != _httpHeaders.end(); element++)
        ret.append(element->first + ": " + element->second + "\n");
    return ret;
}
httpMessage::~httpMessage(void){}
httpMessage::httpMessage(void){}