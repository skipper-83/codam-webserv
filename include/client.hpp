#pragma once

#include <memory>

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
	// Client(std::shared_ptr<AsyncFD> socketFd, std::map<AsyncFD::EventTypes, AsyncFD::EventCallback>& eventCallbacks);
    ~Client();

    Client(const Client &) = delete;
    Client &operator=(const Client &) = delete;

    Client(Client &&);
    Client &operator=(Client &&) = delete;

    AsyncIO &socketFd();
    uint16_t port() const;
// 
	// SocketClientCallback clientReadCb;
    void clientReadCb(AsyncSocketClient& client);
    void clientWriteCb(AsyncSocketClient& client);
	void localReadCb();
	void localWriteCb();


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

	void _registerCallbacks();
	void _returnHttpErrorToClient(int code);
};
