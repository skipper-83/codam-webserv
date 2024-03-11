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
//using SocketClientCallback = std::function<void(AsyncSocketClient&)>;
class Client {
	public:
	using SocketClientCallback = AsyncSocketClient::SocketClientCallback;
    Client(std::shared_ptr<AsyncSocketClient> &socketFd);
    ~Client();

    Client(const Client &rhs);
    Client &operator=(const Client &);


    Client(Client &&);
    // Client &operator=(Client &&) = delete;

    AsyncIO &socketFd() const;
    uint16_t port() const;
// 
	// SocketClientCallback clientReadCb;
	void simpleReadCb(AsyncSocketClient& client);
	void simpleWriteCb(AsyncSocketClient& client);

    void clientReadCb(AsyncSocketClient& client);
    void clientWriteCb(AsyncSocketClient& client);
	void localReadCb();
	void localWriteCb();

	void changeState(ClientState newState);
	void setLastActivityTime();
	std::chrono::time_point<std::chrono::steady_clock> getLastActivityTime() const;


   private:
    ClientState _state;
    std::shared_ptr<AsyncSocketClient> _socketFd;
    uint16_t _port;
	size_t _bytesWrittenCounter;
    httpRequest _request;
	httpResponse _response;
    std::string _clientReadBuffer;
    std::string _clientWriteBuffer;
    std::string _localReadBuffer;
    std::string _localWriteBuffer;
	bool _tmpFlag = false;
	std::chrono::time_point<std::chrono::steady_clock> _lastActivityTime = std::chrono::steady_clock::now();

	void _registerCallbacks();
	void _returnHttpErrorToClient(int code);
};
