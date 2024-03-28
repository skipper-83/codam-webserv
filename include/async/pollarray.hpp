#pragma once

#include <poll.h>

#include <list>
#include <unordered_set>

#include "async/fd.hpp"

class AsyncPollArray {
   public:
    void add(std::weak_ptr<AsyncFD> fd);
    void remove(std::weak_ptr<AsyncFD> fd);

    void cleanup();

    void poll(int timeout);

//tmp
	size_t size() const;

   private:
    std::list<std::weak_ptr<AsyncFD>> _weakFDs;
};
