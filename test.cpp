#include <map>
#include <iostream>
#include <fstream>
#include <sstream>

int	get_params(std::ifstream &file, std::map<std::string, std::string> &request)
{
	std::string input;
	std::string::size_type key_end, val_start, val_end;
	
	while (std::getline(file, input))
	{
		if (input.empty())
		{
			// empty line, body should start now
			// this is a good place to test the input for 
			// mandatory variables
			break ;
		}
		key_end = input.find(':', 0);
		if (key_end == std::string::npos){
			std::cout << "not a key/value\n"; //todo: throw exception ?
			return 0;
		}
		val_start = input.find_first_not_of(' ', key_end + 1);
		if (val_start == std::string::npos){
			std::cout << "no value\n";	//todo: throw exception ?
			return 0;
		}
		val_end = input.find(':', val_start);
		if (val_end != std::string::npos){
			std::cout << "two seperators on single line\n"; //todo: throw exception ?
			return 0;
		}
		request.insert(std::make_pair(input.substr(0, key_end), input.substr(val_start, val_end)));
	}
	return 1;
}

int main(int argc, char **argv) {
	
	
	std::map<std::string, std::string> request;
	std::string input;
	std::stringstream body;
	std::ifstream file;

	if (argc != 2)
		file.open("/dev/stdin");
	else
	{
		file.open(argv[1]);
		if (!file.is_open())
			file.open("/dev/stdin");
	}
	std::getline(file, input);
	std::string::size_type request_type_pos, address_pos, protocol_pos;
	request_type_pos = input.find(' ');
	address_pos = input.find(' ', request_type_pos + 1);
	protocol_pos = input.find(' ', address_pos + 1);
	std::cout << "method: " << input.substr(0, request_type_pos)
	<< "\naddress: " << input.substr(request_type_pos + 1, address_pos - request_type_pos - 1) 
	<< "\nprotocol: " << input.substr(address_pos + 1, request_type_pos - address_pos - 1) << "\n";


	get_params(file, request);
	
	int i = 0;
	for (std::map<std::string, std::string>::const_iterator element = request.begin(); element != request.end(); element++)
	{
		std::cout << element->first << ": " << element->second << "\n";
		i++;
	}
	// std::getline(file, input);
	// input 
	body << file.rdbuf();
	input = body.str();
	std::cout << input;
	// std::cout << "Found " << i << " items\n";
}