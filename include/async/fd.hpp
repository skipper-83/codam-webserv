#pragma once

#include <unistd.h>

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

    AsyncFD(AsyncFD&&) = default;
    AsyncFD& operator=(AsyncFD&&) = default;

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

class AsyncIO : public AsyncFD {
   public:
    AsyncIO(int fd, const std::map<EventTypes, EventCallback>& eventCallbacks = {});
    static std::unique_ptr<AsyncIO> create(int fd, const std::map<EventTypes, EventCallback>& eventCallbacks = {});
    virtual ~AsyncIO();

    AsyncIO(const AsyncIO&) = delete;
    AsyncIO& operator=(const AsyncIO&) = delete;

    AsyncIO(AsyncIO&&) = default;
    AsyncIO& operator=(AsyncIO&&) = default;

    std::string read(size_t size);
    size_t write(std::string& data);

    bool eof() const;

   protected:
    static void _internalInCb(AsyncFD& fd);
    static void _internalOutReadyCb(AsyncFD& fd);
    EventCallback _inCb;
    EventCallback _outCb;

    bool _hasPendingRead;
    bool _hasPendingWrite;
    bool _eof;
};

class AsyncSocketClient : public AsyncIO {
   public:
    using SocketClientCallback = std::function<void(AsyncSocketClient&)>;

    AsyncSocketClient(int fd, uint16_t port, const SocketClientCallback& clientReadReadyCb = {}, const SocketClientCallback& clientWriteReadyCb = {});
    static std::unique_ptr<AsyncSocketClient> create(int fd, uint16_t port, const SocketClientCallback& clientReadReadyCb = {},
                                                     const SocketClientCallback& clientWriteReadyCb = {});
    virtual ~AsyncSocketClient();

    AsyncSocketClient(const AsyncSocketClient&) = delete;
    AsyncSocketClient& operator=(const AsyncSocketClient&) = delete;

    AsyncSocketClient(AsyncSocketClient&&) = default;
    AsyncSocketClient& operator=(AsyncSocketClient&&) = default;

    uint16_t getPort() const;

    void registerReadReadyCb(const SocketClientCallback& cb);
    void registerWriteReadyCb(const SocketClientCallback& cb);

   protected:
    static void _internalReadReadyCb(AsyncFD& fd);
    static void _internalWriteReadyCb(AsyncFD& fd);

    uint16_t _port;
    SocketClientCallback _clientReadReadyCb;
    SocketClientCallback _clientWriteReadyCb;
};

class AsyncSocket : public AsyncFD {
   public:
    using SocketCallback = std::function<void(AsyncSocket&)>;

    AsyncSocket(uint16_t port, const SocketCallback& clientAvailableCb = {}, int backlog = 10);
    static std::unique_ptr<AsyncSocket> create(uint16_t port, const SocketCallback& clientAvailableCb = {}, int backlog = 10);
    virtual ~AsyncSocket();

    AsyncSocket(const AsyncSocket&) = delete;
    AsyncSocket& operator=(const AsyncSocket&) = delete;

    AsyncSocket(AsyncSocket&&) = default;
    AsyncSocket& operator=(AsyncSocket&&) = default;

    bool clientAvailable() const;
    std::unique_ptr<AsyncSocketClient> accept(const AsyncSocketClient::SocketClientCallback& clientReadReadyCb = {},
                                              const AsyncSocketClient::SocketClientCallback& clientWriteReadyCb = {});

   protected:
    static void _internalClientAvailableCb(AsyncFD& fd);
    uint16_t _port;
    SocketCallback _clientAvailableCb;
    int _backlog;
    bool _hasPendingAccept;
};
