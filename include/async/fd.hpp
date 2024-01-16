#pragma once

#include <unistd.h>

#include <functional>
#include <memory>
#include <string>

class AsyncFD {
   public:
    virtual ~AsyncFD();

    AsyncFD(const AsyncFD&) = delete;
    AsyncFD& operator=(const AsyncFD&) = delete;

    AsyncFD(AsyncFD&&) = default;
    AsyncFD& operator=(AsyncFD&&) = default;

    void close();

    virtual void readReadyCb() = 0;
    virtual void writeReadyCb() = 0;
    virtual void errorCb() = 0;

    void poll();

    operator bool() const;

    bool isValid() const;

   protected:
    AsyncFD(int fd);
    AsyncFD();

    void setAsyncFlags();
    friend class AsyncPollArray;
    int _fd;
};

class AsyncIOFD : public AsyncFD {
   public:
    AsyncIOFD(int fd);
    static std::unique_ptr<AsyncIOFD> create(int fd);
    virtual ~AsyncIOFD();

    AsyncIOFD(const AsyncIOFD&) = delete;
    AsyncIOFD& operator=(const AsyncIOFD&) = delete;

    AsyncIOFD(AsyncIOFD&&) = default;
    AsyncIOFD& operator=(AsyncIOFD&&) = default;

    void readReadyCb() override;
    void writeReadyCb() override;
    void errorCb() override;

    bool hasPendingRead = false;

    std::string readBuffer;
    std::string writeBuffer;

    static constexpr size_t READ_CHUNK_SIZE = 4096;
};

class AsyncSocket : public AsyncFD {
   public:
    AsyncSocket(uint16_t port, int backlog = 10);
    static std::unique_ptr<AsyncSocket> create(uint16_t port, int backlog = 10);
    virtual ~AsyncSocket();

    AsyncSocket(const AsyncSocket&) = delete;
    AsyncSocket& operator=(const AsyncSocket&) = delete;

    AsyncSocket(AsyncSocket&&) = default;
    AsyncSocket& operator=(AsyncSocket&&) = default;

    void readReadyCb() override;
    void writeReadyCb() override;
    void errorCb() override;

    bool hasPendingAccept() const;
    std::unique_ptr<AsyncIOFD> accept();

    uint16_t getPort() const;

   private:
    int _port;
    int _backlog;
    bool _hasPendingAccept;
};
