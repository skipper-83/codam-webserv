#pragma once

#include <chrono>

#include "async/in_file.hpp"
#include "async/program.hpp"
#include "async/socket_client.hpp"
#include "file_handler.hpp"
#include "httpMessage/http_request.hpp"
#include "httpMessage/http_response.hpp"
#include "httpMessage/cgi_message.hpp"
#include "session.hpp"
#include "buffer.hpp"

enum class ClientState {
    READY_FOR_INPUT,
    READ_REQUEST,
    READ_BODY,
    BUILDING_RESPONSE,
    WRITE_RESPONSE,
	CGI_WRITE,
	CGI_READ,
	FILE_READ,
    ERROR,
    DONE,
};
// using SocketClientCallback = std::function<void(AsyncSocketClient&)>;
class Client {
   public:
    using SocketClientCallback = AsyncSocketClient::SocketClientCallback;
    Client(std::shared_ptr<AsyncSocketClient> &socketFd, std::function<void(std::weak_ptr<AsyncFD>)> addLocalFdToPollArray,
           WebServSessionList &sessionList);
    ~Client();

    Client(const Client &rhs);
    Client &operator=(const Client &);

    // Client(Client &&);

    AsyncIO &socketFd() const;
    uint16_t port() const;

    void changeState(ClientState newState);
    void setLastActivityTime();
    std::chrono::time_point<std::chrono::steady_clock> getLastActivityTime() const;

   private:
    ClientState _state;
    httpRequest _request;
    httpResponse _response;
    WebServSessionList &_sessionList;
    std::shared_ptr<WebServSession> _session = nullptr;

    std::shared_ptr<AsyncSocketClient> _socketFd;
    std::shared_ptr<InFileHandler> _inputFile = nullptr;
	std::shared_ptr<cgiMessage> _cgiMessage;

    uint16_t _port = 0;

    size_t _bytesWrittenCounter = 0;

    Buffer _clientReadBuffer;
    std::string _clientWriteBuffer;
    std::chrono::time_point<std::chrono::steady_clock> _lastActivityTime = std::chrono::steady_clock::now();

    std::function<void(std::weak_ptr<AsyncFD>)> _addLocalFdToPollArray;
    void _registerCallbacks();
    void _returnHttpErrorToClient(int code, std::string message = "");

    void _clientReadCb(AsyncSocketClient &client);
    void _clientWriteCb(AsyncSocketClient &client);

    void _openFileAndAddToPollArray(std::string path);
    void _readFromFile();
	void _readFromCgi();
	void _handleSession();
	void _buildResponse();

    bool _sessionSet = false;
};
