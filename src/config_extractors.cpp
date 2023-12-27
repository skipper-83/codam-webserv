// #include <algorithm>
// #include <fstream>
// #include <functional>
// #include <iostream>
// #include <istream>

// #include <map>
// #include <sstream>
// #include <vector>

#include "logging.hpp"
#include "config.hpp"

// CPPLog::Instance infoLog = logOut.instance(CPPLog::Level::DEBUG, "parse config");
static CPPLog::Instance infoLog = logOut.instance(CPPLog::Level::INFO, "parse config");

/**
 * @brief  Used if input is expected on single line (ie: [listen 8080;\n])
 * 		  Puts first line of [is] in [line] and checks if it is terminated by a ';'
 * 			if not, throws error.
 * 
 * @param is the input strem
 * @param line reference to line, to be used by caller function
 * @param ref string with refferer for error msg
 */
static void checkTerminator(std::istream& is, std::string &line, std::string ref) {
    getline(is, line);
	if (!is)
        throw(std::invalid_argument("Unexpected input for " + ref));
    if (line[line.size() - 1] != ';')
        throw(std::invalid_argument("Missing terminator ; for " + ref));
}

/**
 * @brief Allowed method extractor. Tests for terminator. Then sets all (default) allowed
 * 		methods to false, and afterward all methods it encounters to true. Throws exception
 * 		when it encounters a non-existing method. If succesful, it sets the default flag to false;
 * 
 * @param is 
 * @param rhs 
 * @return std::istream& 
 */
std::istream& operator>>(std::istream& is, AllowedMethods& rhs) {
	std::string line, word;
	std::stringstream lineStream;

	checkTerminator(is, line, "allowed_methods");
	lineStream.str(line);
	if (!lineStream)
		throw(std::invalid_argument("Unexpected input for allowed_methods"));
	for (auto &it : rhs.methods)
		it.second = false;
	while (lineStream >> word)
	{
		if (word.find(';') != std::string::npos)
		{
			if (word.length() > 1)
				word = word.substr(0, word.length() - 1);

		}
		if (rhs.methods.find(word) == rhs.methods.end())
			throw(std::invalid_argument("Invalid method in allowed_methods: [" + word + "]"));
		rhs.methods[word] = true;
		infoLog << word << " method allowed";
	}
	rhs.defaultValue = false;
	return is;
}

/**
 * @brief Autoindex extractor. Checks for terminator and wheter the word afer
 * 		autoindex is 'on' or 'off'. Sets the state of the autoindex struct accordingly
 * 		and sets the default flag to false.
 * 
 * @param is 
 * @param rhs 
 * @return std::istream& 
 */
std::istream& operator>>(std::istream& is, AutoIndex& rhs) {
    std::string line, word;
	std::stringstream lineStream;

    checkTerminator(is, line, "autoindex");
	lineStream.str(line);
	lineStream >> word;
	if (!lineStream)
		throw(std::invalid_argument("Unexpected input for autoindex"));
    if (word.find("on") != std::string::npos && word.length() < 4) {
        rhs.on = true;
        rhs.defaultValue = false;
		infoLog << "autoindex set to true" << CPPLog::end;
        return is;
    }
    if (word.find("off") != std::string::npos && word.length() < 5) {
        rhs.on = false;
        rhs.defaultValue = false;
		infoLog << "autoindex set to false" << CPPLog::end;
        return is;
    }
    throw(std::invalid_argument("Unexpected input for autoindex: " + word));
}

/**
 * @brief ServerNames extractor. Checks for terminator, then adds every word in 
 * 		the line as a servername for the calling server struct.
 * 
 * @param is 
 * @param rhs 
 * @return std::istream& 
 */
std::istream& operator>>(std::istream& is, ServerNames& rhs) {
    std::string line, word;
    std::stringstream lineStream;

    checkTerminator(is, line, "server_name");
    lineStream.str(line);
    while (lineStream >> word) {
        if (!lineStream)
            throw(std::invalid_argument("Incorrect arguments for server_name"));
		if (word[word.size() - 1] == ';') // don't add the terminator
			word = word.substr(0, word.size() - 1); 
        rhs.name_vec.push_back(word); // right now this doesn't test if the name is in a valid format
    }
    if (rhs.name_vec.size()) {
        infoLog << "names: ";
        for (auto it : rhs.name_vec)
            infoLog << it;
    }
    return is;
};

/**
 * @brief Helper for Server extraction operator. Called when [root] is encountered by 
 * 		the location extraction operator. Currently only checks for terminator and 
 * 		extraction error.
 * 			
 * 
 * @param is 
 * @param rhs 
 */
static void setLocationRoot(std::istream& is, Location& rhs) {
    std::string word;

    is >> rhs.root;  // todo: check this stuff ?
    if (!is)
        throw(std::invalid_argument("Incorrect input for location root"));
    if (rhs.root[rhs.root.size() - 1] == ';')
        rhs.root = rhs.root.substr(0, rhs.root.size() - 1);
    else {
        is >> word;
        if (!is)
            throw(std::invalid_argument("Incorrect input for location root"));
        if (word.find(';') == std::string::npos)
            throw(std::invalid_argument("Missing terminating ; after root"));
    }
    infoLog << "root set to " << rhs.root << CPPLog::end;
}

/**
 * @brief Sets the indices for a location. Currently only checks for terminator and 
 * 			extractor error.
 * 
 * @param is 
 * @param rhs 
 */
static void setLocationIndex(std::istream& is, Location& rhs) {
    std::string word, line;
    std::stringstream lineStream;

    checkTerminator(is, line, "location");
    lineStream.str(line);
    while (lineStream >> word && word[0] != ';') {
        if (!lineStream)
            throw(std::invalid_argument("Incorrect input for location index"));
        rhs.index_vec.push_back(word);  // todo: check this stuff
        if (word[word.size() - 1] == ';') {
            rhs.index_vec[rhs.index_vec.size() - 1] = word.substr(0, word.size() - 1);
            break;
        }
    }
    if (rhs.index_vec.size()) {
        infoLog << "indices: " << CPPLog::end;
        for (auto it : rhs.index_vec)
            infoLog << it << CPPLog::end;
    }
}

/**
 * @brief Location extractor overload. Extracts the referrer (the URI after the location keyword)
 * 		and calls setLocationIndex() and/or setLocationRoot() for indices and root respectively
 * 
 * @param is 
 * @param rhs 
 * @return std::istream& 
 */
std::istream& operator>>(std::istream& is, Location& rhs) {
    std::string word;

    is >> rhs.ref >> word;
    if (!is || rhs.ref.find('{') != std::string::npos)
        throw(std::invalid_argument("Wrong referrer for location"));  // todo check referrer more thoroughly
    if (word.find('{') == std::string::npos)
        throw(std::invalid_argument("Missing opening { in location"));
    infoLog << "Referrer: " << rhs.ref << CPPLog::end;
    while (is >> word) {
        if (!is)
            throw(std::invalid_argument("Wrong input for location"));
        if (word.find('}') != std::string::npos) {
            if (rhs.root == "")
                throw(std::invalid_argument("No root for location"));
            break;
        }
        if (word == "root")
            setLocationRoot(is, rhs);
        if (word == "index")
            setLocationIndex(is, rhs);
    }
    return is;
}

/**
 * @brief Extractor for client_max_body_size
 * 
 * @param is 
 * @param rhs 
 * @return std::istream& 
 */
std::istream& operator>>(std::istream& is, BodySize& rhs) {
    double num;
	// int	multiplier = 1;
    std::stringstream lineStream;
    std::string line, size, sizeNames = "kmg";
    std::map<char, int> sizes = {{'b', 1}, {'k', 1000}, {'m', 1000000}, {'g', 1000000000}};

    checkTerminator(is, line, "client_max_body_size");
    lineStream.str(line);
    lineStream >> num >> size;
	infoLog << "num: " << num;
    if (!lineStream)
        throw(std::invalid_argument("Invalid value for client_max_body_size"));
    if (size[0] != ';' && !sizes[size[0]])
        throw(std::invalid_argument("Invalid size type for client_max_body_size. Allowed: g, m, k, b or no identifier"));
	if (size[0] != ';')
    	num *= sizes[size[0]];
    rhs.value = num;
    rhs.defaultValue = false;
    return is;
}

/**
 * @brief Extractor for listenport
 * 
 * @param is 
 * @param rhs 
 * @return std::istream& 
 */
std::istream& operator>>(std::istream& is, ListenPort& rhs) {
    int num;
    std::stringstream lineStream;
    std::string line, word;

    checkTerminator(is, line, "listen");
	lineStream.str(line);
    lineStream >> num;
    if (!lineStream || num < 0 || num > 65535)
        throw(std::invalid_argument("Invalid value for port"));
    lineStream >> word;
    if (word.find_first_not_of(";") != std::string::npos)
        throw(std::invalid_argument("Invalid value for port"));
    rhs.value = num;
    infoLog << "ListenPort set for " << num << CPPLog::end;
    return is;
}

/**
 * @brief Extractor for Errorpage
 * 
 * @param is 
 * @param rhs 
 * @return std::istream& 
 */
std::istream& operator>>(std::istream& is, ErrorPage& rhs)
{
	std::string line, word;
	std::stringstream lineStream;
	
	checkTerminator(is, line, "error_page");
	lineStream.str(line);
	while (lineStream >> word)
	{
		if (!lineStream)
			throw(std::invalid_argument("Unexpected input for error_page"));
		if (std::all_of(word.begin(), word.end(), ::isdigit) && word.find(';') == std::string::npos)
			rhs.errorNumbers.push_back(stoi(word));
		else
		{
			if (rhs.errorNumbers.size() < 1)
				throw(std::invalid_argument("No error numbers given for error_page"));
			if (word.find(';') != std::string::npos)
				word = word.substr(0, word.length() - 1);
			rhs.page = word;
			break ;
		}
	}
	return is;
}

/**
 * @brief Helper for Server extractor operator: sets the array of lambda function to be called when 
 * 			a keyword is encountered
 * 
 * @param subParsers 
 * @param rhs 
 */
static void	setServerSubparsers(SubParsers &subParsers, ServerConfig& rhs){
	 subParsers = {
        {"listen",
         [&rhs](std::istream& is) {
             ListenPort new_port;
             is >> new_port;
			for (auto it : rhs.ports) // check for duplicate ports, we could also put this in the maine extractor function
				if (it.value == new_port.value)
					throw(std::invalid_argument("Duplicate listening port in server"));
             rhs.ports.push_back(new_port);
         }},
        {"location",
         [&rhs](std::istream& is) {
             Location new_location;
             is >> new_location;
             rhs.locations.push_back(new_location);
         }},
		{"allowed_methods", [&rhs](std::istream& is) { is >> rhs.allowed; }},
        {"server_name", [&rhs](std::istream& is) { is >> rhs.names; }},
        {"autoindex", [&rhs](std::istream& is) { is >> rhs.autoIndex; }},
        {"client_max_body_size", [&rhs](std::istream& is) { is >> rhs.clientMaxBodySize; }},
		{"error_page", [&rhs](std::istream& is) { ErrorPage new_errorPage; is >> new_errorPage; rhs.errorPages.push_back(new_errorPage); }}
	};
}

/**
 * @brief Server extractor operator. 
 * 
 * @param is 
 * @param rhs 
 * @return std::istream& 
 */
std::istream& operator>>(std::istream& is, ServerConfig& rhs) {
    std::string word;
    bool done = false;
    SubParsers subParsers;
	static int rank;
	
	setServerSubparsers(subParsers, rhs);
    is >> word;
    if (word.find('{') == std::string::npos)
        throw(std::invalid_argument("Expected opening { for server"));
    while (is >> word && !(done = (word.find('}') != std::string::npos))) {
        if (subParsers[word])
            subParsers[word](is);
		else
			break ;
    }
    if (done)
        infoLog << "Server parse complete" << CPPLog::end;
	else
		throw(std::invalid_argument("Unexpected input for server: [" + word + "] Missing closing brace '}' ?"));
	if (rhs.ports.size() < 1) // set default port if no port is set
	{
		ListenPort new_port;
		new_port.value = DEFAULT_PORT;
		rhs.ports.push_back(new_port);
	}
	rhs.rank = rank++;
    return is;
}

/**
 * @brief MainConfig extractor operator
 * 
 * @param is 
 * @param rhs 
 * @return std::istream& 
 */
std::istream& operator>>(std::istream& is, MainConfig& rhs) {
    std::string word, line;
	std::string::size_type pos;
    std::cout << word;
	std::stringstream ss;
   	SubParsers 	subParsers = {
        {"server",
         [&rhs](std::istream& is) {
             ServerConfig newServer;
             is >> newServer;
             rhs._servers.push_back(newServer);
         }},
		{"allowed_methods", [&rhs](std::istream& is) { is >> rhs._allowed; }},
        {"client_max_body_size", [&rhs](std::istream& is) { is >> rhs.clientMaxBodySize; }},
        {"autoindex", [&rhs](std::istream& is) { is >> rhs._autoIndex; }}
    };

 	while (getline(is, line)) {
        if ((pos = line.find('#')) != std::string::npos)
            line.erase(pos);
        if (std::all_of(line.begin(), line.end(), [](char c) { return std::isspace(static_cast<unsigned char>(c)); }))
            continue;
        ss << line << "\n";
    }
    while (ss >> word) {
        if (subParsers[word]) {
            subParsers[word](ss);
        }
		else
			throw (std::invalid_argument("Unexpected input in config file: " + word));
    }
	rhs._overrideDefaults();
	rhs._setServerNameAndPortArrays();
    return is;
}

