#pragma once

#include <memory>

#include "async/fd.hpp"
#include "http_request.hpp"
#include "http_response.hpp"

class Client {
   public:
    Client(std::shared_ptr<AsyncFD> fd, uint16_t port);
    ~Client();

    Client(const Client &) = delete;
    Client &operator=(const Client &) = delete;

    Client(Client &&) = default;
    Client &operator=(Client &&) = default;

    AsyncFD &fd();
    uint16_t port() const;

    httpRequest &request();
    httpResponse &response();

   private:
    std::shared_ptr<AsyncFD> _fd;
    uint16_t _port;
    httpRequest _request;
    httpResponse _response;
};
