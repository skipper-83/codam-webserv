#include "buffer.hpp"

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
    std::cout << " Can we get a line? " << buffer.getCRLFLine(line) << std::endl;  // true
    std::cout << "Line got: [" << line << "]" << std::endl;                    // Hello World!\nProep
    buffer.print();
    std::cout << "\tLines: " << buffer.lines() << std::endl;                   // 1
    std::cout << "\tSize: " << buffer.size() << std::endl;                     // 13
    std::cout << " Can we get a line? " << buffer.getCRLFLine(line) << std::endl;  // true
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
    std::cout << " Can we get a line? " << buffer.getCRLFLine(line) << std::endl;  // false

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
	line.clear();
	new_buffer.readAndRemove(5, line);
	std::cout << "Read and remove 5 from buffer: [" << line << "]" << std::endl;
	new_buffer.print();

    return 0;
}