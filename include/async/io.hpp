#pragma once
#include "input.hpp"
#include "output.hpp"

class AsyncIO : public AsyncInput, public AsyncOutput {
   public:
    AsyncIO(int fd, const std::map<EventTypes, EventCallback>& eventCallbacks = {});
    AsyncIO(const std::map<EventTypes, EventCallback>& eventCallbacks = {});
    static std::unique_ptr<AsyncIO> create(int fd, const std::map<EventTypes, EventCallback>& eventCallbacks = {});
    virtual ~AsyncIO();

    AsyncIO(const AsyncIO&) = delete;
    AsyncIO& operator=(const AsyncIO&) = delete;

    AsyncIO(AsyncIO&&) = delete;
    AsyncIO& operator=(AsyncIO&&) = delete;
};
