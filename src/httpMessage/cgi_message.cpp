#include "httpMessage/cgi_message.hpp"

#include "config.hpp"
#include "logging.hpp"

static CPPLog::Instance infoLog = logOut.instance(CPPLog::Level::INFO, "cgi");

cgiMessage::cgiMessage(std::string const& cgiPath, const httpRequest* request, std::function<void(std::weak_ptr<AsyncFD>)> addAsyncFdToPollArray)
    : _request(request), _addAsyncFdToPollArray(addAsyncFdToPollArray) {
	infoLog << "Creating CGI Message" << CPPLog::end;
    _cgi = AsyncProgram::create(cgiPath, _request->getPath(), {}, std::bind(&cgiMessage::_cgiReadCb, this, std::placeholders::_1),
                                std::bind(&cgiMessage::_cgiWriteCb, this, std::placeholders::_1));
    _cgi->addToPollArray(_addAsyncFdToPollArray);
}
void cgiMessage::_cgiReadCb(AsyncProgram& cgi) {
	infoLog << "Checking for EOF" << CPPLog::end;
    if (cgi.eof()) {
		infoLog << "EOF found" << CPPLog::end;
        _bodyComplete = true;
        return;
    }
    _readBuffer += cgi.read(DEFAULT_READ_SIZE);
    infoLog << "read buffer: " << _readBuffer << " size: " << _readBuffer.size() << CPPLog::end;

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

					_readBuffer = headerLine + _readBuffer; // put back the line that was removed
                    infoLog << "Cgi response has no headers. Readbuffer now: " << _readBuffer << CPPLog::end;
                    _headersComplete = true;
					return ;
                }
            }
        }

		if (_readBuffer.find("\r\n\r\n") == std::string::npos || _readBuffer.find("\n\n") == std::string::npos) {
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
    // infoLog << "cgi write callback" << CPPLog::end;
}
