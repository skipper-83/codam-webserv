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
    Client(std::shared_ptr<AsyncSocketClient> &fd, uint16_t port);
	// Client(std::shared_ptr<AsyncFD> fd, std::map<AsyncFD::EventTypes, AsyncFD::EventCallback>& eventCallbacks);
    ~Client();

    Client(const Client &) = delete;
    Client &operator=(const Client &) = delete;

    Client(Client &&) = default;
    Client &operator=(Client &&) = default;

    AsyncIO &fd();
    uint16_t port() const;
// 
	// SocketClientCallback clientReadCb;
    void clientReadCb(AsyncSocketClient& client);
    void clientWriteCb();
	void localReadCb();
	void localWriteCb();


   private:
    ClientState _state;
    std::shared_ptr<AsyncSocketClient> _fd;
    uint16_t _port;
    httpRequest _request;
	httpResponse _response;
    std::string _clientReadBuffer;
    std::string _clientWriteBuffer;
    std::string _localReadBuffer;
    std::string _localWriteBuffer;
};
