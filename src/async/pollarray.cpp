#include "async/pollarray.hpp"

#include <poll.h>

#include <algorithm>
#include <unordered_map>

void AsyncPollArray::add(std::shared_ptr<AsyncFD> fd) {
    _fds.insert(fd);
}

void AsyncPollArray::remove(std::shared_ptr<AsyncFD> fd) {
    _fds.erase(fd);
}

void AsyncPollArray::cleanup() {
    std::erase_if(_fds, [](const std::shared_ptr<AsyncFD>& fd) { return !fd->isValid(); });
}

void AsyncPollArray::poll(int timeout) {
    cleanup();

    std::unordered_map<int, std::shared_ptr<AsyncFD>> fdMap;
    fdMap.reserve(_fds.size());

    std::transform(_fds.begin(), _fds.end(), std::inserter(fdMap, fdMap.end()),
                   [](const std::shared_ptr<AsyncFD>& fd) { return std::make_pair(fd->_fd, fd); });

    std::vector<pollfd> pollfds;
    pollfds.reserve(_fds.size());

    for (const auto& fd : fdMap) {
        pollfds.push_back({fd.first, POLLIN | POLLOUT | POLLERR, 0});
    }

    int ret = ::poll(pollfds.data(), pollfds.size(), timeout);
    if (ret < 0) {
        throw std::runtime_error("poll failed");
    }

    if (ret == 0) {
        return;
    }
    for (const auto& pfd : pollfds) {
        if (pfd.revents & POLLIN) {
            fdMap[pfd.fd]->readReadyCb();
        }
        if (pfd.revents & POLLOUT) {
            fdMap[pfd.fd]->writeReadyCb();
        }
        if (pfd.revents & POLLERR) {
            fdMap[pfd.fd]->errorCb();
        }
    }
}
