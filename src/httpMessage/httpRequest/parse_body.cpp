#include "httpMessage/http_request.hpp"
#include "logging.hpp"

static CPPLog::Instance infoLog = logOut.instance(CPPLog::Level::INFO, "httpRequest body parser");
static CPPLog::Instance warningLog = logOut.instance(CPPLog::Level::WARNING, "httpRequest body parser");

/**
 * @brief Adds from the filestream to the body of the request until the stop condition (depending on header) is encountered
 *
 * @param fs
 */
void httpRequest::parseBody(std::istream &fs) {
    std::streampos nextChunkSize;
    // std::stringstream contents;
    std::string line;
	// std::streampos originalPos = fs.tellg();


    infoLog << "ADD TO BODY: " << _remainingLength(fs) << CPPLog::end;
	// infoLog << "Buffer [" << fs.rdbuf() << "]" << CPPLog::end;
	// fs.seekg(originalPos);
    if (_remainingLength(fs) == 0)
        return;
    if (this->_bodyComplete)
        return;
    if (this->_chunkedRequest)
        return _addChunkedContent(fs);
    if (this->_contentSizeSet)
        return _addToFixedContentSize(fs);
    _addUntilNewline(fs);
}

/**
 * @brief Reads from filestream until it reaches the size set in the header
 *
 * @param fs
 */
void httpRequest::_addToFixedContentSize(std::istream &fs) {
    std::stringstream contents;
    std::streampos addedLength;

    if (this->_contentLength > this->_clientMaxBodySize)
        throw httpRequestException(413, "Request body larger than max body size");
    addedLength = _remainingLength(fs);
    if ((size_t)addedLength + this->_bodyLength >= this->_contentLength) {
        try {
            _httpBody += _readNumberOfBytesFromFileStream(fs, _contentLength - _bodyLength);
        } catch (const std::exception &e) {
            warningLog << "Error reading http request: " << e.what() << CPPLog::end;
            throw httpRequestException(400, "Error reading http request: " + std::string(e.what()));
        }
        this->_bodyLength = _contentLength;
        _bodyComplete = true;
    } else {
        contents << fs.rdbuf();
        this->_httpBody += contents.str();
        this->_bodyLength += addedLength;
    }
    // infoLog << this->_httpBody << " " << this->_bodyLength << CPPLog::end;
    return;
}

/**
 * @brief Add to body from a chunked filestream. Expects the amount of bytes as a number on one line, then that amount of bytes to read on the next.
 * Keeps reading until it encountes 0\n\n.
 *
 * @param fs
 */
void httpRequest::_addChunkedContent(std::istream &fs) {
    std::string line;
    // std::streampos nextChunkSize;

    while (fs) {
        if (!_chunkSizeKnown)  // if we do not have chunk size from the last iteration
        {
			infoLog << "Getting chunk size" << CPPLog::end;
            line = _getLineWithCRLF(fs);
            // MAYBE IMPLEMENT CHECK FOR EMPTY LINE
			if (line.empty())
			{
				infoLog << "Empty line, returning\n";
				return;
			}
            infoLog << "New chunk size: " << line << CPPLog::end;
            try {
                _nextChunkSize = stoi(line, nullptr, 16); // convert hex to int
				_chunkSizeKnown = true;
				infoLog << "Chunk size: " << _nextChunkSize << CPPLog::end;
            } catch (std::exception &e) {
                throw httpRequestException(400, "Incorrect chunk size in chunked http request: " + std::string(e.what()));
            }
        }
		if (this->_bodyLength + _nextChunkSize > this->_clientMaxBodySize)
            throw httpRequestException(413, "Request body larger than max body size");
        if (_nextChunkSize > 0 && _remainingLength(fs) < (_nextChunkSize) + 2) {  // +2 for newline
			return ;
            // throw httpRequestException(400, "Chunk size smaller than announced size in chunked http request");
        }
        
        if (_nextChunkSize == 0) {  // end of body
            line = _getLineWithCRLF(fs);
            if (line.empty()) {
                this->_bodyComplete = true;
                return;
            }
            throw httpRequestException(400, "Terminating line of chunked http request non-empty");
        }
        try {
            this->_httpBody += _readNumberOfBytesFromFileStream(fs, _nextChunkSize);
        } catch (const std::exception &e) {
            throw httpRequestException(400, "Error reading chunked http request: " + std::string(e.what()));
        }
        this->_bodyLength += _nextChunkSize;
		infoLog << "Body size: " << this->_bodyLength << " -- old chunk size: " << _nextChunkSize << CPPLog::end;
        _nextChunkSize = 0;
		_chunkSizeKnown = false;
        line = _getLineWithCRLF(fs);  // skip terminating newline

		// line = _getLineWithCRLF(fs);  // skip terminating newline
        // std::getline(fs, line);  // skip terminating newline
    }
    fs.clear();
    return;
}

/**
 * @brief Reads from filestream until an empty line is encountered.
 *
 * @param fs
 */
void httpRequest::_addUntilNewline(std::istream &fs) {
    std::string line;
    // std::cerr << "Parsing until double newline\n";
    line = _getLineWithCRLF(fs);
    while (fs) {
        // std::cerr << "loop\n";
        // std::cerr << "line: [" << line << "]\n";
        if (line.empty()) {
            std::cerr << "found double nl\n";
            fs.clear();
            this->_bodyComplete = true;
            infoLog << "found double newline: " << this->_httpBody << " size:" << this->_bodyLength << CPPLog::end;
            // NEWLINE TEST 2
            if (_httpBody[_httpBody.size() - 1] != '\n') {
                this->_httpBody += '\n';
                ++this->_bodyLength;
            }
            return;
        }
        if (line.size() + this->_bodyLength + static_cast<int>(fs.eof()) > this->_clientMaxBodySize)
            throw httpRequestException(413, "Request body larger than max body size");
        this->_httpBody += line;
        this->_bodyLength += line.size();
        infoLog << "eof bit: " << fs.eof() << CPPLog::end;
        if (!fs.eof()) {
            this->_httpBody += "\n";
            this->_bodyLength += 1;
        }
        line = _getLineWithCRLF(fs);
    }
    infoLog << "found EOF: " << this->_httpBody << " size:" << this->_bodyLength << CPPLog::end;
    fs.clear();
    return;
}