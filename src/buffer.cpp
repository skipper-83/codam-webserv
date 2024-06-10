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
    for (size_t i = 0; i < data.size(); i++) {
        item.data.push_back(data[i]);
        if (data[i] == '\n' && i > 0 && data[i - 1] == '\r') {
			if (i == 1 || (i - 3 > 0 && data[i - 3] == '\r' && data[i - 2] == '\n'))  // if the line is empty
				this->_emptyLines++;
            this->_linesInBuffer++;
            item.lines++;
        }
    }
	infoLog << "Added " << item.size << " bytes to buffer; lines now: " << this->_linesInBuffer << CPPLog::end;
    this->buffer.push_back(item);
    this->_sizeOfBuffer += data.size();
}

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
                data[i + 1] = '\0';
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
	infoLog << "Getting line from buffer, lines: " << this->_linesInBuffer << CPPLog::end;
	size_t bytesRead = 0;
    if (this->_linesInBuffer == 0)
        return 0;
    for (std::list<BufferItem>::iterator::value_type& item : this->buffer) {
        // if (item.lines > 0) {
            for (size_t i = 0; i < item.size; i++) {
                if (item.data[i] == '\n' && i > 0 && item.data[i - 1] == '\r') {
                    line = line.substr(0, line.size() - 1); // remove the last character
					// this->linesInBuffer--;
					// item.lines--;
                    this->remove(bytesRead + 1);
					// infoLog << "Line got: [" << line << "], lines now: " << this->linesInBuffer << CPPLog::end;
                    return 1;
                }
                line += (char)item.data[i];
				bytesRead++;
            }
        // }
    }
    return 0;
}

