#pragma once
#include "fd.hpp"
#include "socket_client.hpp"

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
