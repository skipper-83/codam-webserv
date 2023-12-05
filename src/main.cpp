#include <iostream>
#include "hello-world.hpp"
#include "http_request.hpp"

int main(int argc, char **argv) {
    std::cout << hello_world() << std::endl;
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
		std::cout << test.getHeader("Host");
		std::cout << test.getHeader("User-Agent");
		std::cout << test.getHeader("Connnection") << "\n\n\ncopy:\n";
		httpRequest::httpRequestListT	cookies = test.getHeaderList("Connection");
		httpRequest::httpRequestListT	empty = test.getHeaderList("fewfewfewf");

		std::cout << cookies.at(0) << "\n" << cookies.at(1) << "\n";
		if (empty.empty())
			std::cout << "\t\tempty\n";
		else
			std::cout << "\t\tno empty\n";

	if (cookies.empty())
			std::cout << "\t\tvooies empty\n";
		else
			std::cout << "\t\tcookies no empty\n";
		httpRequest copy(test);
		std::cout << copy;
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	
    return 0;
}