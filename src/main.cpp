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
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
    return 0;
}