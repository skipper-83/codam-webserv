#pragma once

#include <unordered_set>

#include "async/fd.hpp"

class AsyncPollArray {
   public:
    void add(std::shared_ptr<AsyncFD> fd);
    void remove(std::shared_ptr<AsyncFD> fd);

    void cleanup();

    void poll(int timeout);

   private:
    std::unordered_set<std::shared_ptr<AsyncFD>> _fds;
};
