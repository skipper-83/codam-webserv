#include "httpMessage/cgi_message.hpp"

#include <cstring>

#include "config.hpp"
#include "logging.hpp"

static CPPLog::Instance infoLog = logOut.instance(CPPLog::Level::INFO, "cgi");
static CPPLog::Instance fatalLog = logOut.instance(CPPLog::Level::FATAL, "cgi");

cgiMessage::cgiMessage(std::string const& cgiPath, const httpRequest* request, std::function<void(std::weak_ptr<AsyncFD>)> addAsyncFdToPollArray)
    : _request(request), _addAsyncFdToPollArray(addAsyncFdToPollArray) {
    _makeEnvironment();
    infoLog << "Creating CGI Message" << CPPLog::end;
    _cgi = AsyncProgram::create(cgiPath, std::filesystem::absolute(_request->getPath()), _cgiEnv, std::bind(&cgiMessage::_cgiReadCb, this, std::placeholders::_1),
                                std::bind(&cgiMessage::_cgiWriteCb, this, std::placeholders::_1));
    _cgi->addToPollArray(_addAsyncFdToPollArray);
}

void cgiMessage::_makeEnvironment() {
    _cgiEnv["REDIRECT_STATUS"] = "200";
    _cgiEnv["CONTENT_LENGTH"] = std::to_string(_request->getBodyLength());
    _cgiEnv["CONTENT_TYPE"] = _request->getHeader("Content-Type");
    _cgiEnv["PATH_INFO"] = _request->getPath();
    _cgiEnv["PATH_TRANSLATED"] = _request->getPath();
    _cgiEnv["QUERY_STRING"] = _request->getQueryString();
    _cgiEnv["REQUEST_METHOD"] = WebServUtil::httpMethodToString(_request->getMethod());
    _cgiEnv["SERVER_NAME"] = _request->getHeader("Host");
    _cgiEnv["SERVER_PORT"] = std::to_string(_request->getPort());
    _cgiEnv["SERVER_PROTOCOL"] = _request->getProtocol();
    _cgiEnv["SERVER_SOFTWARE"] = DEFAULT_SERVER_NAME;
    _cgiEnv["HTTP_COOKIE"] = _request->getHeader("Cookie");
    httpMessage::httpRequestT headers = _request->getHeaderMap();
    for (auto header : headers) {
        std::string header_key = header.first;
        if (header_key.size() > 2 && strncmp(header_key.c_str(), "X-", 2) == 0) {
            std::string::size_type i = 0;
            while ((i = header_key.find('-', i)) != std::string::npos) {
                header_key.replace(i, 1, "_");
                i++;
            }
            _cgiEnv["HTTP_" + header_key] = header.second;
        }
    }
}

int cgiMessage::checkProgramStatus() {
    try {
        _cgiIsRunning = _cgi->isRunning();
        if (!_cgiIsRunning) {
            _cgiExitCode = _cgi->getExitCode();
            infoLog << "Cgi program has exited with code: " << _cgiExitCode << CPPLog::end;
            return _cgiExitCode;
        }
        return 0;
    } catch (std::exception& e) {
        infoLog << "Error checking program status: " << e.what() << CPPLog::end;
        return -1;
    }

    return 0;
}

void cgiMessage::_cgiReadCb(AsyncProgram& cgi) {
    // infoLog << "Checking for EOF" << CPPLog::end;
    if (cgi.eof()) {
        infoLog << "EOF found" << CPPLog::end;
        _bodyComplete = true;
        return;
    }
    _readBuffer += cgi.read(DEFAULT_READ_SIZE);
    // infoLog << "read buffer: " << _readBuffer << " size: " << _readBuffer.size() << CPPLog::end;

    if (!_headersComplete) {
        std::pair<std::string, std::string> key_value;
        if (!_cgiHasHeadersInOutput) {
            if ((_readBuffer.find("\r\n") != std::string::npos || _readBuffer.find("\n") != std::string::npos)) {
                infoLog << "Testing for headers in output";
                std::string headerLine;
                try {
                    headerLine = _getLineWithCRLF(_readBuffer);
                    infoLog << "Header line: " << headerLine << CPPLog::end;
                    key_value = _parseHeaderLine(headerLine);
                    setHeader(key_value.first, key_value.second);
                    _cgiHasHeadersInOutput = true;
                } catch (std::exception& e) {
                    _readBuffer = headerLine + _readBuffer;  // put back the line that was removed
                    infoLog << "Cgi response has no headers. Readbuffer now: " << _readBuffer << CPPLog::end;
                    _headersComplete = true;
                    return;
                }
            }
        }
        if (_readBuffer.find("\r\n\r\n") == std::string::npos && _readBuffer.find("\n\n") == std::string::npos) {
            infoLog << "Headers not complete" << CPPLog::end;
            return;
        } else {
            std::string line;
            while (!(line = _getLineWithCRLF(_readBuffer)).empty()) {
                try {
                    key_value = _parseHeaderLine(line);
                    setHeader(key_value.first, key_value.second);
                } catch (std::exception& e) {
                    infoLog << "Error parsing headers from cgi" << e.what() << " line: " << line << CPPLog::end;
                    _headersComplete = true;
                    return;
                }
            }
            _headersComplete = true;
        }
    } else {
        _httpBody += _readBuffer;
        _readBuffer.clear();
    }
}

void cgiMessage::_cgiWriteCb(AsyncProgram& cgi) {
    // (void)_writeBuffer;
    (void)cgi;
    int bytesWritten;
    std::string writeChunk;
    if (_request && _request->getMethod() == WebServUtil::HttpMethod::POST && _writeCounter < _request->getBodyLength()) {
        infoLog << "Writing body to cgi, counter is at: " << _writeCounter << CPPLog::end;
        writeChunk = _request->getBody().substr(_writeCounter, DEFAULT_WRITE_SIZE);
        bytesWritten = cgi.write(writeChunk);
        if (bytesWritten < 0) {
            fatalLog << "Error writing to cgi" << CPPLog::end;
            cgi.kill();
            return;
        }
        _writeCounter += bytesWritten;
    }
}
