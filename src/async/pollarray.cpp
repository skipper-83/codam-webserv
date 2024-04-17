#include "async/pollarray.hpp"

#include <poll.h>

#include <algorithm>
#include <unordered_map>
#include <vector>

#include "logging.hpp"
#include "async/pollarray.hpp"

static CPPLog::Instance logI = logOut.instance(CPPLog::Level::INFO, "AsyncPollArray");
static CPPLog::Instance logD = logOut.instance(CPPLog::Level::DEBUG, "AsyncPollArray");
static CPPLog::Instance logW = logOut.instance(CPPLog::Level::WARNING, "AsyncPollArray");
static CPPLog::Instance logE = logOut.instance(CPPLog::Level::ERROR, "AsyncPollArray");

void AsyncPollArray::add(std::weak_ptr<AsyncFD> fd) {
    _weakFDs.push_back(fd);
}

void AsyncPollArray::remove(std::weak_ptr<AsyncFD> fd) {
    _weakFDs.remove_if([fd](const std::weak_ptr<AsyncFD>& wfd) {
        if (wfd.expired())
            return true;
        return wfd.lock() == fd.lock();
    });
}

void AsyncPollArray::cleanup() {
    _weakFDs.remove_if([](const std::weak_ptr<AsyncFD>& wfd) { return wfd.expired(); });
}

void AsyncPollArray::poll(int timeout) {
    cleanup();
    std::vector<std::shared_ptr<AsyncFD>> fds;
    fds.reserve(_weakFDs.size());
    for (const auto& wfd : _weakFDs) {
        std::shared_ptr<AsyncFD> fd = wfd.lock();
        if (fd && fd->isValid() && !fd->_eventCallbacks.empty()) {
            fds.push_back(fd);
        }
    }

    std::vector<pollfd> pollfds;
    pollfds.reserve(fds.size());

    for (const auto& fd : fds) {
        pollfd pfd;
        pfd.fd = fd->_fd;
        pfd.events = 0;
        pfd.revents = 0;
        for (auto [eventType, cb] : fd->_eventCallbacks) {
            pfd.events |= AsyncFD::eventTypeToPoll.at(eventType);
        }
        pollfds.push_back(pfd);
    }

    int ret = ::poll(pollfds.data(), pollfds.size(), timeout);
    if (ret < 0) {
        logE << "poll failed";
        throw std::runtime_error("poll failed");
    }

    if (ret == 0) {
        return;
    }

    if (fds.size() != pollfds.size()) {
        logE << "fds.size() != pollfds.size()";
        throw std::runtime_error("poll returned different number of fds than expected");
    }

    for (size_t i = 0; i < pollfds.size(); i++) {
        for (auto [event, eventType] : AsyncFD::pollToEventType) {
            if (pollfds[i].revents & event) {
                if (auto cb = fds[i]->_eventCallbacks[eventType]) {
                    cb(*fds[i]);
                }
            }
        }
    }
}

size_t AsyncPollArray::size() const {
    return size_t(_weakFDs.size());
}
