#pragma once

#include <memory>
#include <string>

#include "Connection.hpp"

class Socket : public FileDescriptor {
   public:
    Socket(int port, int backlog = 5);
    Socket(const Socket &other) = delete;
    virtual ~Socket();

    std::unique_ptr<Connection> accept();
    bool pending() const;

    Socket &operator=(const Socket &other) = delete;

   private:
    void _socket();
    void _fcntl();
    void _bind(int port);
    void _listen(int backlog);
};
