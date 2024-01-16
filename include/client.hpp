#pragma once

#include "async/fd.hpp"
#include <memory>

#include "http_request.hpp"

class Client {
   public:
    Client(std::shared_ptr<AsyncIOFD> fd, uint16_t port);
    ~Client();

    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;

    Client(Client&&) = default;
    Client& operator=(Client&&) = default;

    AsyncIOFD &fd();
    uint16_t port() const;

    httpRequest &request();

   private:
    std::shared_ptr<AsyncIOFD> _fd;
    uint16_t _port;
    httpRequest _request;
};
