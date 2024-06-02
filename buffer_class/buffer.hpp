#pragma once

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
	void readAndRemove(size_t length, std::string& data);
    void clear();
    void print(void);
    int getCRLFLine(std::string& line);
    size_t lines(void) { return this->linesInBuffer; };
    size_t size(void) { return this->sizeOfBuffer; };
};