#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>

class AsyncFD {
   public:
    using EventCallback = std::function<void(AsyncFD&)>;
    enum class EventTypes {
        IN,
        OUT,
        ERROR,
        HANGUP,
    };

    AsyncFD();
    AsyncFD(int fd, const std::map<EventTypes, EventCallback>& eventCallbacks = {});
    AsyncFD(const std::map<EventTypes, EventCallback>& eventCallbacks);

    static std::unique_ptr<AsyncFD> create(int fd, const std::map<EventTypes, EventCallback>& eventCallbacks = {});
    static std::unique_ptr<AsyncFD> create(const std::map<EventTypes, EventCallback>& eventCallbacks);
    virtual ~AsyncFD();

    AsyncFD(const AsyncFD&) = delete;
    AsyncFD& operator=(const AsyncFD&) = delete;

    AsyncFD(AsyncFD&&) = delete;
    AsyncFD& operator=(AsyncFD&&) = delete;

    void close();
    void poll();

    operator bool() const;
    bool isValid() const;

   protected:
    friend class AsyncPollArray;
    static const std::map<int, EventTypes> pollToEventType;
    static const std::map<EventTypes, int> eventTypeToPoll;

    void setAsyncFlags();

    void eventCb(EventTypes type);
    int _fd;
    std::map<EventTypes, EventCallback> _eventCallbacks;
};
