#pragma once

// #include <memory>

#include <chrono>

#include "async/fd.hpp"
#include "http_request.hpp"
#include "http_response.hpp"

enum class ClientState {
    READY_FOR_INPUT,
    READ_REQUEST,
    READ_BODY,
    BUILDING_RESPONSE,
    WRITE_RESPONSE,
    ERROR,
    DONE,
};
// using SocketClientCallback = std::function<void(AsyncSocketClient&)>;
class Client {
   public:
    using SocketClientCallback = AsyncSocketClient::SocketClientCallback;
    Client(std::shared_ptr<AsyncSocketClient> &socketFd, std::function<void(std::shared_ptr<AsyncFD>)> addLocalFdToPollArray);
    ~Client();

    Client(const Client &rhs);
    Client &operator=(const Client &);

    Client(Client &&);
    // Client &operator=(Client &&) = delete;

    AsyncIO &socketFd() const;
    // AsyncFile
    uint16_t port() const;
    //
    // SocketClientCallback clientReadCb;
    void simpleReadCb(AsyncSocketClient &client);
    void simpleWriteCb(AsyncSocketClient &client);

    void clientReadCb(AsyncSocketClient &client);
    void clientWriteCb(AsyncSocketClient &client);
    void localReadCb();
    void localWriteCb();

    void changeState(ClientState newState);
    void setLastActivityTime();
    std::chrono::time_point<std::chrono::steady_clock> getLastActivityTime() const;

   private:
    ClientState _state;
    httpRequest _request;
    httpResponse _response;

    std::shared_ptr<AsyncSocketClient> _socketFd;
    std::shared_ptr<AsyncFile> _localFd = nullptr;

    uint16_t _port;
    // std::string _path;

    size_t _bytesWrittenCounter;

    std::string _clientReadBuffer;
    std::string _clientWriteBuffer;
    std::string _localReadBuffer;
    std::string _localWriteBuffer;
    std::chrono::time_point<std::chrono::steady_clock> _lastActivityTime = std::chrono::steady_clock::now();

    std::function<void(std::shared_ptr<AsyncFD>)> _addLocalFdToPollArray;
    void _registerCallbacks();
    void _returnHttpErrorToClient(int code);
    std::string _resolvePath();
};
