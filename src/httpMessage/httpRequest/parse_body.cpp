#include "httpMessage/http_request.hpp"
#include "logging.hpp"

static CPPLog::Instance infoLog = logOut.instance(CPPLog::Level::INFO, "httpRequest body parser");
static CPPLog::Instance warningLog = logOut.instance(CPPLog::Level::WARNING, "httpRequest body parser");

/**
 * @brief Adds from the filestream to the body of the request until the stop condition (depending on header) is encountered
 *
 * @param fs
 */
void httpRequest::parseBody(Buffer &input) {
    // std::streampos nextChunkSize;
    // std::stringstream contents;
    std::string line;
    // std::streampos originalPos = fs.tellg();

    infoLog << "ADD TO BODY: " << input.size() << CPPLog::end;
    // infoLog << "Buffer [" << fs.rdbuf() << "]" << CPPLog::end;
    // fs.seekg(originalPos);
    if (input.size() == 0)
        return;
    if (this->_bodyComplete)
        return;
    if (this->_chunkedRequest)
        return _addChunkedContent(input);
    if (this->_contentSizeSet)
        return _addToFixedContentSize(input);
    _addUntilNewline(input);
}

/**
 * @brief Reads from filestream until it reaches the size set in the header
 *
 * @param fs
 */
void httpRequest::_addToFixedContentSize(Buffer &input) {
    if (this->_contentLength > this->_clientMaxBodySize)
        throw httpRequestException(413, "Request body larger than max body size");
    // addedLength = input.size();
    if (input.size() + this->_bodyLength >= this->_contentLength) {
        try {
            input.readAndRemove(this->_contentLength - this->_bodyLength, this->_httpBody);
            // _readNumberOfBytesFromFileStream(fs, _contentLength - _bodyLength);
        } catch (const std::exception &e) {
            warningLog << "Error reading http request: " << e.what() << CPPLog::end;
            throw httpRequestException(400, "Error reading http request: " + std::string(e.what()));
        }
        this->_bodyLength = _contentLength;
        _bodyComplete = true;
    } else {
        this->_bodyLength += input.size();
        input.readAndRemove(input.size(), this->_httpBody);
    }
    return;
}

/**
 * @brief Add to body from a chunked filestream. Expects the amount of bytes as a number on one line, then that amount of bytes to read on the next.
 * Keeps reading until it encountes 0\n\n.
 *
 * @param fs
 */
void httpRequest::_addChunkedContent(Buffer &input) {
    std::string line;

    while (input.getCRLFLine(line)) {
        if (!_chunkSizeKnown)  // if we do not have chunk size from the last iteration
        {
            infoLog << "Getting chunk size" << CPPLog::end;
            if (line.empty()) {
                infoLog << "Empty line, returning\n";
                return;
            }
            infoLog << "New chunk size: " << line << CPPLog::end;
            try {
                _nextChunkSize = stoi(line, nullptr, 16);  // convert hex to int
                _chunkSizeKnown = true;
                infoLog << "Chunk size: " << _nextChunkSize << CPPLog::end;
                line.clear();
                continue;
            } catch (std::exception &e) {
                throw httpRequestException(400, "Incorrect chunk size in chunked http request: " + std::string(e.what()));
            }
        }
        // throw exception if body size is larger than max body size
        if (this->_bodyLength + _nextChunkSize > this->_clientMaxBodySize)
            throw httpRequestException(413, "Request body larger than max body size");

        // if the chunk size is 0, the body is complete and we can return
        if (_nextChunkSize == 0) {  // end of body
            if (line.empty()) {
                this->_bodyComplete = true;
                return;
            }
            throw httpRequestException(400, "Terminating line of chunked http request non-empty");
        }
        infoLog << "Reading chunk size: " << _nextChunkSize << " line size: " << line.size() << CPPLog::end;

        // if chunksize is know and not 0, read the chunksize amount of bytes from the buffer
        this->_httpBody += line;
        this->_bodyLength += line.size();
        infoLog << "Body size: " << this->_bodyLength << " -- old chunk size: " << _nextChunkSize << CPPLog::end;
        _nextChunkSize = 0;
        _chunkSizeKnown = false;
        line.clear();
    }
    line.clear();
    return;
}

/**
 * @brief Reads from filestream until an empty line is encountered.
 *
 * @param fs
 */
void httpRequest::_addUntilNewline(Buffer &input) {
    std::string line;
    if (input.lines() == 0 && input.size() != 0) {
        input.readAndRemove(input.size(), this->_httpBody);
        this->_bodyLength += input.size();
        return;
    }
    if (!this->_firstNewLineFound) {
        input.getCRLFLine(line);
        this->_httpBody += line;
        this->_bodyLength += line.size();
        this->_firstNewLineFound = true;
        line.clear();
    }
    if (input.lines() != 0) {
        input.getCRLFLine(line);
        this->_bodyComplete = true;
        return;
    }

    return;
}
