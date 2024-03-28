#pragma once

#include <limits>

#include "async/in_file.hpp"

class InFileHandler {
   public:
    InFileHandler(std::string path, std::size_t bufferSize);
    ~InFileHandler();

    InFileHandler(const InFileHandler&) = delete;
    InFileHandler& operator=(const InFileHandler&) = delete;

    InFileHandler(InFileHandler&&) = delete;
    InFileHandler& operator=(InFileHandler&&) = delete;

    operator std::shared_ptr<AsyncFD>() const;

	std::shared_ptr<AsyncInFile> getFD() const;

    std::string read(std::size_t size = std::numeric_limits<std::size_t>::max());
    bool readBufferEmpty() const;
    std::size_t readBufferLength() const;
    std::size_t readBufferSpaceLeft() const;
    bool readBufferFull() const;
    bool eof() const;
    bool bad() const;

   private:
    void _readCb(AsyncInFile& file);

    std::shared_ptr<AsyncInFile> _fd;
    std::size_t _bufferSize;
    std::string _readbuffer;
    bool _bad;
    bool _eof;
};