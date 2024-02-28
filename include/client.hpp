#pragma once

#include <memory>

#include "async/fd.hpp"
#include "http_request.hpp"

enum class ClientState {
    READY_FOR_INPUT,
	READ_REQUEST,
	READ_BODY,
	BUILDING_RESPONSE,
    WRITE_RESPONSE,
    ERROR,
    DONE,
};

class Client {
   public:
    Client(std::shared_ptr<AsyncIOFD> fd, uint16_t port);
    ~Client();

    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;

    Client(Client&&) = default;
    Client& operator=(Client&&) = default;

    // AsyncIOFD &fd();
    // uint16_t port() const;

<<<<<<< Updated upstream
    httpRequest &request();
=======
    void clientReadCb();
    void clientWriteCb();
>>>>>>> Stashed changes

   private:
    ClientState _state;
    std::shared_ptr<AsyncIOFD> _fd;
    uint16_t _port;
    httpRequest _request;
<<<<<<< Updated upstream
=======
    httpResponse _response;

    std::string _clientReadBuffer;
    std::string _clientWriteBuffer;
    std::string _localReadBuffer;
    std::string _localWriteBuffer;
>>>>>>> Stashed changes
};
