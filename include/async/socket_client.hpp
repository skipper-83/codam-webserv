#pragma once
#include "io.hpp"

class AsyncSocketClient : public AsyncIO {
   public:
    using SocketClientCallback = std::function<void(AsyncSocketClient&)>;

    AsyncSocketClient(int fd, uint16_t port, const SocketClientCallback& clientReadReadyCb = {}, const SocketClientCallback& clientWriteReadyCb = {});
    static std::unique_ptr<AsyncSocketClient> create(int fd, uint16_t port, const SocketClientCallback& clientReadReadyCb = {},
                                                     const SocketClientCallback& clientWriteReadyCb = {});
    virtual ~AsyncSocketClient();

    AsyncSocketClient(const AsyncSocketClient&) = delete;
    AsyncSocketClient& operator=(const AsyncSocketClient&) = delete;

    AsyncSocketClient(AsyncSocketClient&&) = delete;
    AsyncSocketClient& operator=(AsyncSocketClient&&) = delete;

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
