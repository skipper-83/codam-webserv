#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <istream>
#include <map>
#include <functional>

// void     CPPLog(std::Str)

template <char C>
std::istream &expect(std::istream &s) {
    if (s.flags() & std::ios_base::skipws) s >> std::ws;
    if (s.peek() == C)
        s.ignore();
    else
        s.setstate(std::ios_base::failbit);
    return (s);
}

struct ServerConfig
{

};

struct BodySize
{
	int value;
};

struct MainConfig
{
    std::vector<ServerConfig> servers;
    bool autoIndex;
    std::vector<std::string> serverNames;
	BodySize clientMaxBodySize;
};



std::istream& operator>>(std::istream& is, BodySize &lhs)
{
	
	int num;
	std::stringstream lineStream;
	std::string line, size, sizeNames = "kmg";
	std::map<char, int> sizes = {
		{'k', 1},
		{'m', 1000},
		{'g', 1000000}
	};

	getline(is, line);
	lineStream.str(line);
	lineStream >> num  >> size;
	if (!is)
		throw (std::invalid_argument("Invalid value for client body size limit"));
	if (line[line.length() - 1] != ';')
		throw (std::invalid_argument("No terminator for limit"));
	if (!sizes[size[0]])
		throw (std::invalid_argument("Invalid size character for client body size limit"));
	num *= sizes[size[0]];
	lhs.value = num;
	return is;
}

std::istream& operator>>(std::istream& is, ServerConfig &lhs)
{
	(void)is;
	(void)lhs;
    std::cout << "server!";
	return is;
}

std::istream& operator>>(std::istream& is, MainConfig &lhs)
{
    (void)lhs;
    (void)is;
    std::string word;
    std::cout << word;
    std::map<std::string, std::function<void (std::istream &)> > sub_parsers = 
    {
        {"server", [&lhs](std::istream& is){ ServerConfig newServer; is >> newServer; lhs.servers.push_back(newServer); }},
		{"client_max_body_size", [&lhs](std::istream& is){ is >> lhs.clientMaxBodySize; }}

        // {"server", [&lhs](std::istream& is){ ServerConfig newServer; is >> newServer; lhs.servers.push_back(newServer); }
    };

    while (is >> word)
    {
		if(sub_parsers[word])
		{
			sub_parsers[word](is);
		}
    }
    return is;
}

int main(int argc, char** argv)
{
    std::fstream file;
    std::string line;
    std::string::size_type pos;
    std::stringstream ss;

    if (argc != 2)
        return 1;
    file.open(argv[1]);
    if (!file)
        return 1;
    while (getline(file, line))
    {
        if ((pos = line.find('#')) != std::string::npos)
            line.erase(pos);
        if (std::all_of(line.begin(), line.end(), [](char c){ return std::isspace(static_cast<unsigned char>(c));} ))
            continue;
        ss << line << "\n";
    }
    // std::cout << ss.str();
    MainConfig test;
    ss >> test;
	std::cout << "Servers: " << test.servers.size() << "\n";
	std::cout << "Max size: " << test.clientMaxBodySize.value << "\n";
    // std::cout << "BOOH";
}