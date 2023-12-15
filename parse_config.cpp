#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <istream>
#include <map>
#include <sstream>
#include <vector>

#include "logging.hpp"
#include "parse_config.hpp"

CPPLog::Instance l = logOut.instance(CPPLog::DEBUG, "parse config");
CPPLog::Instance lInfo = logOut.instance(CPPLog::INFO, "parse config");

// template <char C>
// std::istream& expect(std::istream& s) {
//     if (s.flags() & std::ios_base::skipws)
//         s >> std::ws;
//     if (s.peek() == C)
//         s.ignore();
//     else
//         s.setstate(std::ios_base::failbit);
//     return (s);
// }

//GET|POST|PUT|DELETE|HEAD|OPTIONS|PATCH



static void checkTerminator(std::istream& is, std::string line, std::string ref) {
    if (!is)
        throw(std::invalid_argument("Unexpected input for " + ref));
    if (line[line.size() - 1] != ';')
        throw(std::invalid_argument("Missing terminator ; for " + ref));
}

std::istream& operator>>(std::istream& is, AllowedMethods& lhs) {
	std::string line, word;
	std::stringstream lineStream;

	getline(is, line);
	checkTerminator(is, line, "allowed_methods");
	lineStream.str(line);
	if (!lineStream)
		throw(std::invalid_argument("Unexpected input for allowed_methods"));
	for (auto it : lhs.methods)
		it.second = false;
	while (lineStream >> word)
	{
		if (word.find(';') != std::string::npos)
		{
			if (word.length() > 1)
				word = word.substr(0, word.length() - 1);

		}
		if (!lhs.methods[word])
			throw(std::invalid_argument("Invalid method in allowed_methods"));
		lhs.methods[word] = true;
		l << word << " method allowed";
	}
	lhs.defaultValue = false;
	return is;
}

std::istream& operator>>(std::istream& is, AutoIndex& lhs) {
    std::string line, word;
	std::stringstream lineStream;

    getline(is, line);
    checkTerminator(is, line, "autoindex");
	lineStream.str(line);
	lineStream >> word;
	if (!lineStream)
		throw(std::invalid_argument("Unexpected input for autoindex"));
    if (word.find("on") != std::string::npos && word.length() < 4) {
        lhs.on = true;
        lhs.defaultValue = false;
		l << "autoindex set to true" << CPPLog::end;
        return is;
    }
    if (word.find("off") != std::string::npos && word.length() < 5) {
        lhs.on = false;
        lhs.defaultValue = false;
		l << "autoindex set to false" << CPPLog::end;
        return is;
    }
    throw(std::invalid_argument("Unexpected input for autoindex"));
}

std::istream& operator>>(std::istream& is, ServerNames& lhs) {
    std::string line, word;
    std::stringstream lineStream;

    getline(is, line);
    checkTerminator(is, line, "server_name");
    lineStream.str(line);
    while (lineStream >> word) {
        if (!lineStream)
            throw(std::invalid_argument("Incorrect arguments for server_name"));
        lhs.name_vec.push_back(word);
    }
    if (lhs.name_vec.size()) {
        l << "names: ";
        for (auto it : lhs.name_vec)
            l << it;
    }
    return is;
};

static void setLocationRoot(std::istream& is, Location& lhs) {
    std::string word;

    is >> lhs.root;  // todo: check this stuff
    if (!is)
        throw(std::invalid_argument("Incorrect input for location root"));
    if (lhs.root[lhs.root.size() - 1] == ';')
        lhs.root = lhs.root.substr(0, lhs.root.size() - 1);
    else {
        is >> word;
        if (!is)
            throw(std::invalid_argument("Incorrect input for location root"));
        if (word.find(';') == std::string::npos)
            throw(std::invalid_argument("Missing terminating ; after root"));
    }
    l << "root set to " << lhs.root << CPPLog::end;
}

static void setLocationIndex(std::istream& is, Location& lhs) {
    std::string word, line;
    std::stringstream lineStream;

    getline(is, line);
    lineStream.str(line);
    checkTerminator(is, line, "location");
    while (lineStream >> word && word[0] != ';') {
        if (!lineStream)
            throw(std::invalid_argument("Incorrect input for location index"));
        lhs.index_vec.push_back(word);  // todo: check this stuff
        if (word[word.size() - 1] == ';') {
            lhs.index_vec[lhs.index_vec.size() - 1] = word.substr(0, word.size() - 1);
            break;
        }
    }
    if (lhs.index_vec.size()) {
        l << "indices: " << CPPLog::end;
        for (auto it : lhs.index_vec)
            l << it << CPPLog::end;
    }
}

std::istream& operator>>(std::istream& is, Location& lhs) {
    std::string word;

    is >> lhs.ref >> word;
    if (!is || lhs.ref.find('{') != std::string::npos)
        throw(std::invalid_argument("Wrong referrer for location"));  // todo check referrer mor thoroughly
    if (word.find('{') == std::string::npos)
        throw(std::invalid_argument("Missing opening { in location"));
    l << "Referrer: " << lhs.ref << CPPLog::end;
    while (is >> word) {
        if (!is)
            throw(std::invalid_argument("Wrong input for location"));
        if (word.find('}') != std::string::npos) {
            if (lhs.root == "")
                throw(std::invalid_argument("No root for location"));
            break;
        }
        if (word == "root")
            setLocationRoot(is, lhs);
        if (word == "index")
            setLocationIndex(is, lhs);
    }
    return is;
}

std::istream& operator>>(std::istream& is, BodySize& lhs) {
    float num;
    std::stringstream lineStream;
    std::string line, size, sizeNames = "kmg";
    std::map<char, int> sizes = {{'k', 1}, {'m', 1000}, {'g', 1000000}};

    getline(is, line);
    if (line[line.length() - 1] != ';')
        throw(std::invalid_argument("No terminator for limit"));
    lineStream.str(line);
    lineStream >> num >> size;
    if (!lineStream)
        throw(std::invalid_argument("Invalid value for client body size limit"));
    if (!sizes[size[0]])
        throw(std::invalid_argument("Invalid size character for client body size limit"));
    num *= sizes[size[0]];
    lhs.value = num;
    lhs.defaultValue = false;
    return is;
}

std::istream& operator>>(std::istream& is, ListenPort& lhs) {
    int num;
    std::stringstream lineStream;
    std::string line, word;

    l << "ListenPort stream extraction operator called" << CPPLog::end;
    getline(is, line);
    if (line[line.length() - 1] != ';')
        throw(std::invalid_argument("No terminator for port"));
    lineStream.str(line);
    lineStream >> num;
    if (!lineStream || num < 0 || num > 65535)
        throw(std::invalid_argument("Invalid value for port"));
    lineStream >> word;
    if (word.find_first_not_of(";") != std::string::npos)
        throw(std::invalid_argument("Invalid value for port"));
    lhs.value = num;
    l << "ListenPort set for " << num << CPPLog::end;
    return is;
}

std::istream& operator>>(std::istream& is, ServerConfig& lhs) {
    std::string word;
    bool done = false;
    std::map<std::string, std::function<void(std::istream&)> > sub_parsers = {
        {"listen",
         [&lhs](std::istream& is) {
             ListenPort new_port;
             is >> new_port;
			for (auto it : lhs.ports)
				if (it.value == new_port.value)
					throw(std::invalid_argument("Duplicate listening port in server"));
             lhs.ports.push_back(new_port);
         }},
        {"location",
         [&lhs](std::istream& is) {
             Location new_location;
             is >> new_location;
             lhs.locations.push_back(new_location);
         }},
		{"allowed_methods", [&lhs](std::istream& is) { is >> lhs.allowed; }},
        {"server_name", [&lhs](std::istream& is) { is >> lhs.names; }},
        {"autoindex", [&lhs](std::istream& is) { is >> lhs.autoIndex; }},
        {"client_max_body_size", [&lhs](std::istream& is) { is >> lhs.clientMaxBodySize; }}};
    (void)lhs;

    is >> word;
    if (word.find('{') == std::string::npos)
        throw(std::invalid_argument("Expected opening { for server"));
    while (is >> word && !(done = (word.find('}') != std::string::npos))) {
        if (sub_parsers[word]) {
            sub_parsers[word](is);
        }
    }
    if (done)
        l << "Server parse complete" << CPPLog::end;
    std::cout << "server!";
    return is;
}

std::istream& operator>>(std::istream& is, MainConfig& lhs) {
    (void)lhs;
    (void)is;
    std::string word;
    std::cout << word;
    std::map<std::string, std::function<void(std::istream&)> > sub_parsers = {
        {"server",
         [&lhs](std::istream& is) {
             ServerConfig newServer;
             is >> newServer;
             lhs.servers.push_back(newServer);
         }},
        {"client_max_body_size", [&lhs](std::istream& is) { is >> lhs.clientMaxBodySize; }},
        {"autoindex", [&lhs](std::istream& is) { is >> lhs.autoIndex; }}
    };

    while (is >> word) {
        if (sub_parsers[word]) {
            sub_parsers[word](is);
        }
    }
    return is;
}

int main(int argc, char** argv) {
    std::fstream file;
    std::string line;
    std::string::size_type pos;
    std::stringstream ss;

    if (argc != 2)
        return 1;
    file.open(argv[1]);
    if (!file)
        return 1;
    while (getline(file, line)) {
        if ((pos = line.find('#')) != std::string::npos)
            line.erase(pos);
        if (std::all_of(line.begin(), line.end(), [](char c) { return std::isspace(static_cast<unsigned char>(c)); }))
            continue;
        ss << line << "\n";
    }
    // std::cout << ss.str();
    MainConfig test;
    ss >> test;
    std::cout << "Servers: " << test.servers.size() << "\n";
    std::cout << "Max size: " << test.clientMaxBodySize.value << "\n";
    l << "test" << CPPLog::end;
}