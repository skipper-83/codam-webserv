#include "buffer.hpp"
#include "logging.hpp"

static CPPLog::Instance infoLog = logOut.instance(CPPLog::Level::INFO, "buffer");

/**
 * @brief Operator += adds data to the end of the buffer
 * 
 * @param data the data to add
 */
void Buffer::operator+=(const std::string& data) {
	this->add(data);
}

/**
 * @brief Operator = replaces the contents of the buffer buffer with the data
 * 
 * @param data the data to replace the buffer with
 */
void Buffer::operator=(const std::string& data) {
        this->clear();
        this->add(data);
}

/**
 * @brief Adds data to the end of the buffer
 * 
 * @param data the data to add
 */
void Buffer::add(const std::string& data) {
    BufferItem item;
    item.size = data.size();
    //     infoLog << "Data to add: " << data << CPPLog::end;
    //     infoLog << "Size of data " << data.size() << CPPLog::end;
	// infoLog << "lines now: " << this->_linesInBuffer << "\nemptylines now: " << this->_emptyLines << CPPLog::end;
    // infoLog << "Buffer now: " << this->read((this->size()));
    for (size_t i = 0; i < data.size(); i++) {
        item.data.push_back(data[i]);
		switch (data[i]) {
			case '\r':
			switch (_newLineStatus)
			{
			case CRLF:
				_newLineStatus = CRLFCR;
				break;
			
			case NO_NEWLINE:
				_newLineStatus = CR;
				break;
			
			default:
				_newLineStatus = NO_NEWLINE;
			}
			break ;

			case '\n':
			switch (_newLineStatus)
			{
			case CR:
				_linesInBuffer++;
				item.lines++;
				_newLineStatus = CRLF;
				break;

			case CRLFCR:
				_linesInBuffer++;
				item.lines++;
				_emptyLines++;
				break;
			
			default:
				_newLineStatus = NO_NEWLINE;
				break;
			}
			break ;

			default:
				_newLineStatus = NO_NEWLINE;
		}

    }
    this->_sizeOfBuffer += data.size();
    this->buffer.push_back(item);
    // infoLog << "Data added: " << data << CPPLog::end;
	// infoLog << "Added " << item.size << " bytes to buffer; lines now: " << this->_linesInBuffer << "\nemptylines now: " << this->_emptyLines << CPPLog::end;
    // infoLog << "Buffer now: " << this->read((this->size()));
}

// GET /directory HTTP/1.1  
// Host: localhost:8081  
// User-Agent: Go-http-client/1.1  
// Accept-Encoding: gzip  
  

/**
 * @brief Returns a string of length bytes from the buffer, leaves the buffer unchanged
 * 
 * @param length length of the string to return, if length is greater than the buffer size, the whole buffer is returned
 * @return std::string 
 */
std::string Buffer::read(size_t length) {
    std::string data;
	size_t bytesRead = 0;
    for (std::list<BufferItem>::iterator::value_type& item : this->buffer) {
        for (size_t i = 0; i < item.size; i++) {
            data += (char)item.data[i];
            length--;
			bytesRead++;
            if (length == 0 || bytesRead == this->_sizeOfBuffer) {
                // data[i + 1] = '\0';
                return data;
            }
        }
    }
    return data;
}

/**
 * @brief Reads length bytes from the buffer and stores them in data
 * 
 * @param length the number of bytes to read
 * @param data the string to store the data in
 * @return size_t the number of bytes read
 */
size_t Buffer::read(size_t length, std::string& data) {
	data = this->read(length);
	return data.size();
}

/**
 * @brief Reads length bytes from the buffer and stores them in data, then removes the data from the buffer
 * 
 * @param length the number of bytes to read
 * @param data the string to store the data in
 */
void Buffer::readAndRemove(size_t length, std::string& data) {
	data = this->read(length);
	this->remove(data.size());
}

/**
 * @brief Prints the buffer to the console
 * 
 */
void Buffer::print(void) {
    for (std::list<BufferItem>::iterator::value_type& item : this->buffer) {
        for (size_t i = 0; i < item.size; i++) {
            std::cout << (char)item.data[i];
        }
    }
    std::cout << std::endl;
}

/**
 * @brief Removes length bytes from the beginning of the buffer
 * 
 * @param length the number of bytes to remove
 */
void Buffer::remove(size_t length) {
	infoLog << "Removing " << length << " bytes from buffer" << CPPLog::end;
    if (length >= this->_sizeOfBuffer) {
        this->clear();
		infoLog << "Buffer cleared, size now: " << this->_sizeOfBuffer << CPPLog::end;
        return;
    }
    while (length > 0) {
        BufferItem& item = this->buffer.front();
        if (item.size <= length) {
            length -= item.size;
            this->_sizeOfBuffer -= item.size;
            this->_linesInBuffer -= item.lines;
            this->buffer.pop_front();
        } else {
            item.data.erase(item.data.begin(), item.data.begin() + length);
            item.size -= length;
            this->_sizeOfBuffer -= length;
            this->_linesInBuffer -= item.lines;
            item.lines = 0;
			infoLog << "Lines in buffer: " << this->_linesInBuffer << "; re-adding lines now" << CPPLog::end;
            for (size_t i = 0; i < item.size; i++) {
                if (item.data[i] == '\n' && i > 0 && item.data[i - 1] == '\r') {
                    this->_linesInBuffer++;
                    item.lines++;
                }
            }
			infoLog << "Lines in buffer: " << this->_linesInBuffer << "; re-added lines" << CPPLog::end;
            length = 0;
        }
    }
}

char Buffer::operator[](size_t index) {
	size_t currentLength = 0;

	for (auto item : this->buffer) {
		if (index < currentLength + item.size)
			return item.data[index - currentLength];
		currentLength += item.size;
	}
    return 0;
}

/**
 * @brief Clears the buffer
 * 
 */
void Buffer::clear() {
    this->buffer.clear();
    this->_sizeOfBuffer = 0;
    this->_linesInBuffer = 0;
	this->_emptyLines = 0;
}

/**
 * @brief Gets a line from the buffer
 * 
 * @param line the line to return
 * @return int 1 if a line was found, 0 otherwise
 */
int Buffer::getCRLFLine(std::string& line) {
	infoLog << "Getting line from buffer, lines: " << this->_linesInBuffer << " size: " << this->_sizeOfBuffer <<  CPPLog::end;
	size_t bytesRead = 0;
    if (this->_linesInBuffer == 0)
        return 0;
    for (std::list<BufferItem>::iterator::value_type& item : this->buffer) {
        // if (item.lines > 0) {
            for (size_t i = 0; i < item.size; i++) {
                if (item.data[i] == '\n' && bytesRead > 0 && line[bytesRead - 1] == '\r') {
                    line = line.substr(0, line.size() - 1); // remove the last character
                    // line[i + 1] = '\0';
					// this->linesInBuffer--;
					// item.lines--;
                    this->remove(bytesRead + 1);
					// infoLog << "Line got: [" << line << "], lines now: " << this->linesInBuffer << CPPLog::end;
                    // line[]
                    return 1;
                }
                line += (char)item.data[i];
				bytesRead++;
            }
        // }
    }
    return 0;
}

bool BufferItem::endsWithCRLF() {
    if(data.size() > 1 && data[data.size() - 1] == '\n' && data[data.size() - 2] == '\r')
        return true;
    return false;
}

bool BufferItem::endsWithCR() {
    if(data.size() > 0 && data[data.size() - 1] == '\r')
        return true;
    return false;
}
