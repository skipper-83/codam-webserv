#include "async/pollarray.hpp"

#include <poll.h>

#include <algorithm>
#include <unordered_map>
#include <vector>

#include "async/pollarray.hpp"
#include "logging.hpp"

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
    _weakFDs.remove_if([](const std::weak_ptr<AsyncFD>& wfd) {
        std::shared_ptr<AsyncFD> fd = wfd.lock();
        return !fd || !fd->isValid();
    });
}

void AsyncPollArray::poll(int timeout) {
    cleanup();
    std::vector<std::shared_ptr<AsyncFD>> fds;
    fds.reserve(_weakFDs.size());
    std::transform(_weakFDs.begin(), _weakFDs.end(), std::back_inserter(fds), [](const std::weak_ptr<AsyncFD>& wfd) { return wfd.lock(); });

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

    fds.clear();

    int ret = ::poll(pollfds.data(), pollfds.size(), timeout);
    if (ret < 0) {
        logE << "poll failed";
        throw std::runtime_error("poll failed");
    }

    if (ret == 0) {
        return;
    }

    if (_weakFDs.size() != pollfds.size()) {
        logE << "fds.size() != pollfds.size()";
        std::cout << "fds.size() != pollfds.size(), fds.size() = " << fds.size() << ", pollfds.size() = " << pollfds.size()
                  << ", weakFDs.size() = " << _weakFDs.size() << "\n";
        throw std::runtime_error("poll returned different number of fds than expected");
    }

    std::list<std::weak_ptr<AsyncFD>>::iterator it = _weakFDs.begin();
    for (auto& pfd : pollfds) {
        for (auto [event, eventType] : AsyncFD::pollToEventType) {
            if (pfd.revents & event) {
                std::shared_ptr<AsyncFD> fd = it->lock();
                if (!fd) {
                    it++;
                    continue;
                }
                if (auto cb = fd->_eventCallbacks[eventType]) {
                    cb(*fd);
                }
            }
        }
        it++;
    }
}

size_t AsyncPollArray::size() const {
    return size_t(_weakFDs.size());
}
