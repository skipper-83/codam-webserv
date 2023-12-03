
#include "include/http_request.hpp"



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


int main(int argc, char **argv) {
	std::ifstream file;

	if (argc != 2)
		file.open("/dev/stdin");
	else
	{
		file.open(argv[1]);
		if (!file.is_open())
			file.open("/dev/stdin");
	}
	try
	{
		httpRequest test(file);
		std::cout << test;
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
}

