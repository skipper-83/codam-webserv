#include <filesystem>
#include <regex>

#include "httpMessage/http_request.hpp"
#include "logging.hpp"

static CPPLog::Instance infoLog = logOut.instance(CPPLog::Level::INFO, "httpRequest header parser");
static CPPLog::Instance warningLog = logOut.instance(CPPLog::Level::WARNING, "httpRequest header parser");

void httpRequest::parseHeader(Buffer &input) {
	// infoLog << "Parsing header, bufer is: " << input.read(input.size()) << CPPLog::end;
    _parseHttpStartLine(input);
    _parseHttpHeaders(input);
    this->_headerParseComplete = true;
    infoLog << "Request header parse succesfully";
}

void httpRequest::_parseHttpStartLine(Buffer &input) {
    std::string line;

    // line = _getLineWithCRLF(fs);
    // while (fs && line.empty()) // skip empty lines before request
    //     line = _getLineWithCRLF(fs);
    infoLog << "Buffer: " << input.read(input.size());
    // line = _getLineWithCRLF(fs);
    // while (fs && line.empty()) // skip empty lines before request
    //     line = _getLineWithCRLF(fs);
	while (line.empty()) // skip empty lines before request
    {
        infoLog << "empty line";
		if (!input.getCRLFLine(line))
			return; // no line to parse
    }
    std::string::size_type request_type_pos, address_pos;
    infoLog << "Parsing Start Line: [" << line << "]";
    static const std::regex http_startline_pattern(
        "^(GET|POST|PUT|DELETE|HEAD|OPTIONS|PATCH) "
        "((https?://[\\w-]+(\\.[\\w-]+)*/?)?/((?!/\\.\\.)[^ ]*?)(\\?[^ ]+?)?) "
        "HTTP/\\d\\.\\d$");
    if (!std::regex_match(line, http_startline_pattern))
        throw httpRequestException(400, "Invalid HTTP start line");
    request_type_pos = line.find(' ');
    address_pos = line.find(' ', request_type_pos + 1);
    this->_httpMethod = WebServUtil::stringToHttpMethod(line.substr(0, request_type_pos));
    this->_httpAdress = line.substr(request_type_pos + 1, address_pos - request_type_pos - 1);
    if (_httpAdress.find('?') != std::string::npos) {
        _queryString = _httpAdress.substr(_httpAdress.find('?') + 1);
        _httpAdress = _httpAdress.substr(0, _httpAdress.find('?'));
    }
    this->_httpProtocol = line.substr(address_pos + 1, request_type_pos - address_pos - 1);
    infoLog << "Start Line parsed" << CPPLog::end;
    return;
}

void httpRequest::_parseHttpHeaders(Buffer &input) {
    std::string line;
    std::pair<std::string, std::string> key_value;
	int lineParsed = 0;

    while ((lineParsed = input.getCRLFLine(line))) {
        if (line.empty())
		{
			infoLog << "Empty line, end of headers" << CPPLog::end;
            break;
		}
        try {
			infoLog << "Parsing header line: " << line << CPPLog::end;
            key_value = _parseHeaderLine(line);
			line.clear();
        } catch (const std::exception &e) {
            throw httpRequestException(400, "Invalid HTTP header");
        }
        this->setHeader(key_value.first, key_value.second);
        if (key_value.first == "Cookie") {
            parseCookieHeader(key_value.second);
        }
        infoLog << "Header: " << key_value.first << ": " << key_value.second << CPPLog::end;
    }
	if (!lineParsed) // no line to parse
	 	return;
    _checkHttpHeaders();
    _setVars();
    return;
}

void httpRequest::_checkHttpHeaders(void) {
    httpRequestT::iterator host_it;

    static const std::regex http_host_header_pattern(
        "(^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\\-]*[a-zA-Z0-9])\\.)*"
        "([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\\-]*[A-Za-z0-9])"
        "(:[0-9]{1,5})?$)|"
        "(^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}"
        "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(:[0-9]{1,5})?$)");
    host_it = _httpHeaders.find("Host");
    if (host_it == this->_httpHeaders.end() && this->_httpProtocol == "HTTP/1.1")
        throw httpRequestException(400, "No host specified");
    if (host_it != this->_httpHeaders.end() && !std::regex_match(host_it->second, http_host_header_pattern))
        throw httpRequestException(400, "Invalid host specified");
    return;
}

/**
 * @brief Set private vars according to headers
 *  - content_length
 *  - chunked (default: false)
 *  - strip port number from host line
 *
 */
void httpRequest::_setVars(void) {
    std::string var;
    std::string::size_type pos;
    httpRequestT::iterator it;

    if (!(var = getHeader("Content-Length")).empty()) {
        try {
            this->_contentLength = stoi(var);
        } catch (const std::exception &e) {
            warningLog << e.what() << ": wrong argument for Content-Length" << CPPLog::end;
            this->_contentLength = 0;
        }
        this->_contentSizeSet = true;
    }

    if (!(var = getHeader("Transfer-Encoding")).empty() && var == "chunked") {
        this->_chunkedRequest = true;
        this->_contentLength = 0;
    }

    if ((it = this->_httpHeaders.find("Host")) != this->_httpHeaders.end() && (pos = it->second.find(':')) != std::string::npos) {
        var = it->second;
        this->_httpHeaders.erase(it);
        this->setHeader("Host", var.substr(0, pos));
        infoLog << var.substr(pos + 1, var.size() - pos - 1) << CPPLog::end;
        this->_port = stoi(var.substr(pos + 1, var.size() - pos - 1));
    }
}

void httpRequest::_resolvePathAndLocationBlock(void) {
    std::string path;

    _pathSet = true;
    infoLog << "Resolving path for " << this->_httpAdress << CPPLog::end;
	std::string locationResolvePath = this->_httpAdress;
	   if (locationResolvePath.find('.') == std::string::npos && locationResolvePath[locationResolvePath.size() - 1] != '/')  //
        locationResolvePath += '/';
	
    for (auto &location : this->_server->locations) {
        if (location.ref == locationResolvePath.substr(0, location.ref.size())) {
            infoLog << "Matched location: " << location.ref << CPPLog::end;
			if (_httpAdress.size() > location.ref.size())
				path = location.root + this->_httpAdress.substr(location.ref.size(), this->_httpAdress.size());
			else
				path = location.root;
            infoLog << path << CPPLog::end;
            _path = path;
            _location = &location;
            infoLog << "Client max body size: " << location.clientMaxBodySize.value << " bytes" << CPPLog::end;
            _clientMaxBodySize = location.clientMaxBodySize.value;

            infoLog << "Path: " << _path << " Location: " << location.ref << " Root: " << location.root;
            // resolve path if it is a directory
            Cgi const *cgi;
            if (!std::filesystem::exists(_path) &&
                !(_httpMethod == WebServUtil::HttpMethod::PUT) &&  // if the file does not exist and the method is not PUT
                ((cgi = this->_server->getCgiFromPath(_path)) == nullptr ||
                 cgi->allowed.methods.find(_httpMethod) == cgi->allowed.methods.end())) {  // and the path is not a cgi
                infoLog << "File not found, returning 404: " << _path << CPPLog::end;
                throw(httpRequestException(404, "File not found"));
            }
            if (std::filesystem::is_directory(_path)) {
                infoLog << "Path is a directory, request: [" << _httpAdress << "] ref: [" << _location->ref << "]" << CPPLog::end;
				if (_httpAdress[_httpAdress.size() - 1] != '/')
				{  //
        			_httpAdress += '/';
					_path += '/';
				}

                /*
                 * I used to throw a redirect here, but the intra tester expects the server to add a trailing slash to the path
                 */
                // if (path[path.size() - 1] != '/')  // if the path does not end with a slash, redirect
                //     throw(httpRequestException(301, _httpAdress + '/'));
                if (_server->autoIndex.on) {
                    infoLog << "Autoindex is on" << CPPLog::end;
                    _returnAutoIndex = true;
                    _path = path;
                    return;
                }
                if (!_location->index_vec.empty()) {
                    infoLog << "checking for index files in config" << CPPLog::end;
                    for (auto &rootIndexFile : _location->index_vec) {
                            infoLog << "checking" << _path + rootIndexFile;
                        if (std::filesystem::exists(_path + rootIndexFile)) {
                            _path = _path + rootIndexFile;
                            return;
                        }
                    }
                    throw(httpRequestException(404, "Directory index file not found, and autoindex is off"));
                    // }
                }
                infoLog << "No index files found, checking if autoindex is on" << CPPLog::end;

                infoLog << "Autoindex is off, returning 403 Forbidden" << CPPLog::end;
                throw(httpRequestException(403, "No directory index, and autoindex is off"));
            }
            return;
        }
    }
    infoLog << "No location block found, returning 404" << CPPLog::end;
    throw(httpRequestException(404, "No location block found"));
}
void httpRequest::parseCookieHeader(std::string cookieHeader) {
    size_t pos = 0;
    while (pos < cookieHeader.size()) {
        // Find the next semicolon to separate cookies
        size_t semicolonPos = cookieHeader.find(';', pos);
        if (semicolonPos == std::string::npos) {
            semicolonPos = cookieHeader.size();
        }

        // Extract the cookie substring
        std::string cookie = cookieHeader.substr(pos, semicolonPos - pos);

        // Find the position of the equal sign to split into name and value
        size_t equalPos = cookie.find('=');
        if (equalPos != std::string::npos) {
            std::string name = cookie.substr(0, equalPos);
            std::string value = cookie.substr(equalPos + 1);
            // Remove leading and trailing whitespaces from name and value
            name.erase(0, name.find_first_not_of(" \t\r\n"));
            name.erase(name.find_last_not_of(" \t\r\n") + 1);
            value.erase(0, value.find_first_not_of(" \t\r\n"));
            value.erase(value.find_last_not_of(" \t\r\n") + 1);

            _cookies[name] = value;
        }
        pos = semicolonPos + 1;
    }
}