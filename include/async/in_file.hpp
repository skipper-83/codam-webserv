#pragma once
#include "input.hpp"

class AsyncInFile : public AsyncInput {
   public:
    using AsyncInFileCallback = std::function<void(AsyncInFile&)>;

    AsyncInFile(const std::string& path, const AsyncInFileCallback& cb = nullptr);
    static std::unique_ptr<AsyncInFile> create(const std::string& path, const AsyncInFileCallback& cb = nullptr);
    virtual ~AsyncInFile();

    AsyncInFile(const AsyncInFile&) = delete;
    AsyncInFile& operator=(const AsyncInFile&) = delete;

    AsyncInFile(AsyncInFile&&) = delete;
    AsyncInFile& operator=(AsyncInFile&&) = delete;

   protected:
    static void _internalInCb(AsyncFD& fd);
    AsyncInFileCallback _inCb;
};
