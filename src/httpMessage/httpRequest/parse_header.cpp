#include <filesystem>
#include <regex>

#include "http_request.hpp"
#include "logging.hpp"

static CPPLog::Instance infoLog = logOut.instance(CPPLog::Level::INFO, "httpRequest header parser");
static CPPLog::Instance warningLog = logOut.instance(CPPLog::Level::WARNING, "httpRequest header parser");

void httpRequest::parseHeader(std::istream &fs) {
    _parseHttpStartLine(fs);
    _parseHttpHeaders(fs);
    this->_headerParseComplete = true;
    infoLog << "Request header parse succesfully";
}

void httpRequest::_parseHttpStartLine(std::istream &fs) {
    std::string line;

    line = _getLineWithCRLF(fs);
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
    this->_httpMethod = line.substr(0, request_type_pos);
    this->_httpAdress = line.substr(request_type_pos + 1, address_pos - request_type_pos - 1);
    this->_httpProtocol = line.substr(address_pos + 1, request_type_pos - address_pos - 1);
    infoLog << "Start Line parsed" << CPPLog::end;
    return;
}

void httpRequest::_parseHttpHeaders(std::istream &fs) {
    std::string line;
    std::string::size_type key_end, val_start;

    while (fs) {
        line = _getLineWithCRLF(fs);
        if (line.empty())
            break;
        key_end = line.find(':', 0);
        if (key_end == std::string::npos)
            throw httpRequestException(400, "Invalid HTTP key/value pair");
        val_start = line.find_first_not_of(' ', key_end + 1);
        if (val_start == std::string::npos)
            val_start = line.size();
        this->setHeader(line.substr(0, key_end), line.substr(val_start, line.size() - 1));
        // this->_httpHeaders.insert(std::make_pair(line.substr(0, key_end), line.substr(val_start, line.size() - 1)));
    }
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

    infoLog << "Resolving path for " << this->_httpAdress << CPPLog::end;
    for (auto &location : this->_server->locations) {
        if (location.ref == this->_httpAdress.substr(0, location.ref.size())) {
            infoLog << "Matched location: " << location.ref << CPPLog::end;
            path = location.root + this->_httpAdress.substr(1, this->_httpAdress.size());
            infoLog << path << CPPLog::end;
            _path = path;
            _location = &location;

            // resolve path if it is a directory
			if (!std::filesystem::exists(path) && !(_httpMethod == "PUT"))
				throw(httpRequestException(404, "File not found"));
            if (std::filesystem::is_directory(path)) {
                infoLog << "Path is a directory, request: [" << _httpAdress << "] ref: [" << _location->ref << "]" << CPPLog::end;
				if (path[path.size() - 1] != '/') // if the path does not end with a slash, redirect
					throw(httpRequestException(301, _httpAdress + '/'));
                if (_httpAdress == _location->ref)  // if the requested adress is the location root
                {
                    if (!_location->index_vec.empty()) {
                        infoLog << "checking for index files in config" << CPPLog::end;
                        for (auto &rootIndexFile : _location->index_vec) {
                            if (std::filesystem::exists(path + rootIndexFile)) {
                                _path = path + rootIndexFile;
                                return;
                            }
                        }
                    }
                }
				infoLog << "No index files found, checking if autoindex is on" << CPPLog::end;
				if (_server->autoIndex.on) {
					infoLog << "Autoindex is on" << CPPLog::end;
					_returnAutoIndex = true;
					_path = path;
					return;
				}
				infoLog << "Autoindex is off, returning 403 Forbidden" << CPPLog::end;
				throw(httpRequestException(403, "No directory index, and autoindex is off"));
            }
            return;
        }
    }
    infoLog << "No match. Resolved default path: "
            << "." + this->_httpAdress << CPPLog::end;
    _path = DEFAULT_ROOT + this->_httpAdress.substr(1, this->_httpAdress.size());
}