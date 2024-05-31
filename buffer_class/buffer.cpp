#include <iostream>
#include <list>
#include <string>
#include <vector>

struct BufferItem {
    std::vector<int> data;
    size_t size = 0;
    size_t lines = 0;
};

class Buffer {
   private:
    std::list<BufferItem> buffer;
    size_t sizeOfBuffer = 0;
    size_t linesInBuffer = 0;

   public:
    void operator+=(const std::string& data);
    void operator=(const std::string& data);
    void add(const std::string& data);
    void remove(size_t length);
    std::string read(size_t length);
	size_t read(size_t length, std::string& data);
    void clear();
    void print(void);
    int getLine(std::string& line);
    size_t lines(void) { return this->linesInBuffer; };
    size_t size(void) { return this->sizeOfBuffer; };
};


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
            this->linesInBuffer++;
            item.lines++;
        }
    }
    this->buffer.push_back(item);
    this->sizeOfBuffer += data.size();
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
    for (std::__1::list<BufferItem>::iterator::value_type& item : this->buffer) {
        for (size_t i = 0; i < item.size; i++) {
            data += (char)item.data[i];
            length--;
			bytesRead++;
            if (length == 0 || bytesRead == this->sizeOfBuffer) {
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
 * @brief Prints the buffer to the console
 * 
 */
void Buffer::print(void) {
    for (std::__1::list<BufferItem>::iterator::value_type& item : this->buffer) {
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
    if (length >= this->sizeOfBuffer) {
        this->clear();
        return;
    }
    while (length > 0) {
        BufferItem& item = this->buffer.front();
        if (item.size <= length) {
            length -= item.size;
            this->sizeOfBuffer -= item.size;
            this->linesInBuffer -= item.lines;
            this->buffer.pop_front();
        } else {
            item.data.erase(item.data.begin(), item.data.begin() + length);
            item.size -= length;
            this->sizeOfBuffer -= length;
            this->linesInBuffer -= item.lines;
            item.lines = 0;
            for (size_t i = 0; i < length; i++) {
                if (item.data[i] == '\n' && i > 0 && item.data[i - 1] == '\r') {
                    this->linesInBuffer++;
                    item.lines++;
                }
            }
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
    this->sizeOfBuffer = 0;
    this->linesInBuffer = 0;
}

/**
 * @brief Gets a line from the buffer
 * 
 * @param line the line to return
 * @return int 1 if a line was found, 0 otherwise
 */
int Buffer::getLine(std::string& line) {
    if (this->linesInBuffer == 0)
        return 0;
    for (std::__1::list<BufferItem>::iterator::value_type& item : this->buffer) {
        if (item.lines > 0) {
            for (size_t i = 0; i < item.size; i++) {
                if (item.data[i] == '\n' && i > 0 && item.data[i - 1] == '\r') {
                    line = line.substr(0, line.size() - 1);
                    this->remove(i + 1);
                    return 1;
                }
                line += (char)item.data[i];
            }
        }
    }
    return 0;
}

int main(void) {
    Buffer buffer;
    buffer.add("Hello World!\nProep\r\n");
    buffer.print();
    std::cout << "\tLines: " << buffer.lines() << std::endl;  // 1
    std::cout << "\tSize: " << buffer.size() << std::endl;    // 20
    buffer.add("Second line\r\n");
    buffer.print();
    std::cout << "\tLines: " << buffer.lines() << std::endl;  // 2
    std::cout << "\tSize: " << buffer.size() << std::endl;    // 33
    std::string line;
    std::cout << " Can we get a line? " << buffer.getLine(line) << std::endl;  // true
    std::cout << "Line got: [" << line << "]" << std::endl;                    // Hello World!\nProep
    buffer.print();
    std::cout << "\tLines: " << buffer.lines() << std::endl;                   // 1
    std::cout << "\tSize: " << buffer.size() << std::endl;                     // 13
    std::cout << " Can we get a line? " << buffer.getLine(line) << std::endl;  // true
    std::cout << "Line got: [" << line << "]" << std::endl;                    // Second line
    buffer.print();
    std::cout << "\tLines: " << buffer.lines() << std::endl;  // 0
    std::cout << "\tSize: " << buffer.size() << std::endl;    // 0

    std::cout << "\n\n";

    buffer.add("123456\r\n789");
    buffer.print();
    std::cout << "\tLines: " << buffer.lines() << std::endl;  // 1
    std::cout << "\tSize: " << buffer.size() << std::endl;    // 11
    buffer.remove(5);
    buffer.print();
    std::cout << "\tLines: " << buffer.lines() << std::endl;  // 1
    std::cout << "\tSize: " << buffer.size() << std::endl;    // 6
    buffer.remove(2);
    buffer.print();
    std::cout << "\tLines: " << buffer.lines() << std::endl;                   // 0
    std::cout << "\tSize: " << buffer.size() << std::endl;                     // 4
    std::cout << " Can we get a line? " << buffer.getLine(line) << std::endl;  // false

    std::cout << "\n\n";
    buffer.clear();
    buffer.add("Imma read me some");
    buffer.print();
    std::cout << "[" << buffer.read(5) << "]" << std::endl;
    buffer.print();

    std::cout << "\n\n";

    Buffer new_buffer;
    new_buffer = "New buffer with operator= overload";
    new_buffer.print();
    new_buffer += " and some more data";
    new_buffer.print();
    new_buffer = "New data in same buffer";
    new_buffer.print();
	std::cout << "Read too many bytes: [" << new_buffer.read(100) << "]" << "Whole buffer is returned" << std::endl;
	new_buffer.clear();
	line.clear();
	size_t bytes_read =	new_buffer.read(100, line);
	std::cout << "Read from empty buffer: [" << line << "]" << " Bytes read: " << bytes_read << std::endl;
	new_buffer.add("123456\r\n789");
	bytes_read = new_buffer.read(100, line);
	std::cout << "Read 100 from buffer: [" << line << "]" << " Bytes read: " << bytes_read << std::endl;
	line.clear();
	bytes_read = new_buffer.read(5, line);
	std::cout << "Read 5 from buffer: [" << line << "]" << " Bytes read: " << bytes_read << std::endl;

    return 0;
}