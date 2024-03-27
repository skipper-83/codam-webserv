#pragma once
#include "fd.hpp"

class AsyncInput : virtual public AsyncFD {
   public:
    AsyncInput(int fd, const std::map<EventTypes, EventCallback>& eventCallbacks = {});
    AsyncInput(const std::map<EventTypes, EventCallback>& eventCallbacks = {});
    static std::unique_ptr<AsyncInput> create(int fd, const std::map<EventTypes, EventCallback>& eventCallbacks = {});
    virtual ~AsyncInput();

    AsyncInput(const AsyncInput&) = delete;
    AsyncInput& operator=(const AsyncInput&) = delete;

    AsyncInput(AsyncInput&&) = delete;
    AsyncInput& operator=(AsyncInput&&) = delete;

    std::string read(size_t size);
    bool hasPendingRead() const;
    bool eof() const;

   protected:
    static void _internalInCb(AsyncFD& fd);
    EventCallback _inCb;
    bool _hasPendingRead;
    bool _eof;
};
