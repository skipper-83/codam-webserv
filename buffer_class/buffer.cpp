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
    void add(const std::string& data);
    void remove(size_t length);
    void clear();
    void print(void);
    int getLine(std::string& line);
    size_t lines(void) { return this->linesInBuffer; };
    size_t size(void) { return this->sizeOfBuffer; };
};

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

void Buffer::print(void) {
    for (auto& item : this->buffer) {
        for (size_t i = 0; i < item.size; i++) {
            std::cout << (char)item.data[i];
        }
    }
}

void Buffer::remove(size_t length) {
    if (length > this->sizeOfBuffer) {
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

void Buffer::clear() {
    this->buffer.clear();
    this->sizeOfBuffer = 0;
    this->linesInBuffer = 0;
}

int Buffer::getLine(std::string& line) {
	// clear line before adding new data ??
	line.clear();

    if (this->linesInBuffer == 0) {
        return 0;
    }
    for (std::__1::list<BufferItem>::iterator::value_type& item : this->buffer) {
        if (item.lines > 0) {
            for (size_t i = 0; i < item.size; i++) {
                if (item.data[i] == '\n' && i > 0 && item.data[i - 1] == '\r') {
                    line[i - 1] = '\0';
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
    std::cout << "\tLines: " << buffer.lines() << std::endl;
    std::cout << "\tSize: " << buffer.size() << std::endl;
    buffer.add("Second line\r\n");
    buffer.print();
    std::cout << "\tLines: " << buffer.lines() << std::endl;
    std::cout << "\tSize: " << buffer.size() << std::endl;
    std::string line;
    std::cout << " Can we get a line? " << buffer.getLine(line) << std::endl;
    std::cout << "Line got: [" << line << "]" << std::endl;
    buffer.print();
    std::cout << "\tLines: " << buffer.lines() << std::endl;
    std::cout << "\tSize: " << buffer.size() << std::endl;
    std::cout << " Can we get a line? " << buffer.getLine(line) << std::endl;
    std::cout << "Line got: [" << line << "]" << std::endl;
    buffer.print();
    std::cout << "\tLines: " << buffer.lines() << std::endl;
    std::cout << "\tSize: " << buffer.size() << std::endl;

	buffer.add("123456\r\n789");
	buffer.print();
	std::cout << "\tLines: " << buffer.lines() << std::endl;
	std::cout << "\tSize: " << buffer.size() << std::endl;
	buffer.remove(5);
	buffer.print();
	std::cout << "\tLines: " << buffer.lines() << std::endl;
	std::cout << "\tSize: " << buffer.size() << std::endl;
	buffer.remove(2);
	buffer.print();
	std::cout << "\tLines: " << buffer.lines() << std::endl;
	std::cout << "\tSize: " << buffer.size() << std::endl;
	std::cout << " Can we get a line? " << buffer.getLine(line) << std::endl;


	return 0;
}