
#include "http_request.hpp"

httpRequest::httpRequest(std::ifstream &fs)
{
	std::stringstream	body_stream;

	_getHttpStartLine(fs);
	_getHttpHeaders(fs);
	body_stream << fs.rdbuf();
	_httpRequestBody = body_stream.str();
}

httpRequest::httpRequest(const httpRequest &src) 
{
	*this = src;
}

httpRequest::~httpRequest()
{
}

httpRequest &httpRequest::operator=(const httpRequest &rhs) 
{
	if (this == &rhs)
		return *this;
	this->_httpRequestType = rhs._httpRequestType;
	this->_httpAdress = rhs._httpAdress;
	this->_httpProtocol = rhs._httpProtocol;
	this->_httpHeaders = std::map<std::string, std::string>(rhs._httpHeaders);
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

void httpRequest::printHeaders(std::ostream &os) const {
	for (std::map<std::string, std::string>::const_iterator element = _httpHeaders.begin(); 
		element != _httpHeaders.end(); 
		element++)
	os << element->first << ": " << element->second << "\n";
}

void	httpRequest::_getHttpStartLine(std::ifstream &fs)
{
	std::string	line;
	
	std::getline(fs, line);
	std::string::size_type request_type_pos, address_pos;

	std::regex http_startline_pattern(
		"^(GET|POST|PUT|DELETE|HEAD|OPTIONS|PATCH) "
		"((https?://[\\w-]+(\\.[\\w-]+)*/?)?/[^ ]*?(\\?[^ ]+?)?) "
		"HTTP/\\d\\.\\d$"
	);
	if (!std::regex_match(line, http_startline_pattern))
		throw std::invalid_argument("Invalid HTTP start line");
	request_type_pos = line.find(' ');
	address_pos = line.find(' ', request_type_pos + 1);
	_httpRequestType = line.substr(0, request_type_pos);
	_httpAdress = line.substr(request_type_pos + 1, address_pos - request_type_pos - 1) ;
	_httpProtocol = line.substr(address_pos + 1, request_type_pos - address_pos - 1);
	return ;
}

void	httpRequest::_getHttpHeaders(std::ifstream &fs)
{
	std::string line;
	std::string::size_type key_end, val_start, val_end;
	
	while (std::getline(fs, line))
	{
		if (line.empty())
			break ;
		key_end = line.find(':', 0);
		if (key_end == std::string::npos)
			throw std::invalid_argument("Invalid HTTP key/value pair");
		val_start = line.find_first_not_of(' ', key_end + 1);
		if (val_start == std::string::npos)
			throw std::invalid_argument("Empty HTTP key/value pair");
		val_end = line.find(':', val_start);
		if (val_end != std::string::npos)
			throw std::invalid_argument("Invalid HTTP key/value pair: two seperators on single line");
		_httpHeaders.insert(std::make_pair(line.substr(0, key_end), line.substr(val_start, val_end)));
	}
	_checkHttpHeaders();
	return ;
}

void httpRequest::_checkHttpHeaders(void)
{
	std::regex http_host_header_pattern(
		"(^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\\-]*[a-zA-Z0-9])\\.)*"
		"([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\\-]*[A-Za-z0-9])"
		"(:[0-9]{1,5})?$)|"
		"(^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}"
		"(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(:[0-9]{1,5})?$)"
	);
	if (_httpHeaders.find("Host") == _httpHeaders.end() && _httpProtocol == "HTTP/1.1")
		throw std::invalid_argument("No Host specified");
	if (_httpHeaders.find("Host") != _httpHeaders.end() && !std::regex_match(_httpHeaders["Host"], http_host_header_pattern))
		throw std::invalid_argument("Invalid Host specified");
	return ;
}

std::ostream &operator<<(std::ostream &os, httpRequest const &t) {
    os << "Protocol:" << t.getProtocol() << "\n"
	<< "Type: " << t.getRequestType() << "\n" 
	<< "Adress: " << t.getAdress() << "\n";
	t.printHeaders(os);
	os << "\n" << t.getBody() << "\n";
	return os;
}

