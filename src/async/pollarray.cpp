#include "async/pollarray.hpp"

#include <poll.h>

#include <algorithm>
#include <unordered_map>
#include <vector>

#include "logging.hpp"

static CPPLog::Instance logI = logOut.instance(CPPLog::Level::INFO, "AsyncPollArray");
static CPPLog::Instance logD = logOut.instance(CPPLog::Level::DEBUG, "AsyncPollArray");
static CPPLog::Instance logW = logOut.instance(CPPLog::Level::WARNING, "AsyncPollArray");
static CPPLog::Instance logE = logOut.instance(CPPLog::Level::ERROR, "AsyncPollArray");

void AsyncPollArray::add(std::shared_ptr<AsyncFD> fd) {
    _fds.insert(fd);
}

void AsyncPollArray::remove(std::shared_ptr<AsyncFD> fd) {
    _fds.erase(fd);
}

void AsyncPollArray::cleanup() {
    std::erase_if(_fds, [](const std::shared_ptr<AsyncFD>& fd) { 
		if (!fd->isValid()) {
			logI << "removing invalid fd" << CPPLog::end;
		}
		return !fd->isValid();
		});
}

void AsyncPollArray::poll(int timeout) {	
    cleanup();

    std::unordered_map<int, std::shared_ptr<AsyncFD>> fdMap;
    fdMap.reserve(_fds.size());

    std::transform(_fds.begin(), _fds.end(), std::inserter(fdMap, fdMap.end()),
                   [](const std::shared_ptr<AsyncFD>& fd) { return std::make_pair(fd->_fd, fd); });

    std::vector<pollfd> pollfds;
    pollfds.reserve(_fds.size());

    for (auto [fd, asyncFD] : fdMap) {
        pollfd pfd;
        pfd.fd = fd;
        pfd.events = 0;
        pfd.revents = 0;
        for (auto [eventType, cb] : asyncFD->_eventCallbacks) {
            pfd.events |= AsyncFD::eventTypeToPoll.at(eventType);
        }
        pollfds.push_back(pfd);
    }

    int ret = ::poll(pollfds.data(), pollfds.size(), timeout);
    if (ret < 0) {
        throw std::runtime_error("poll failed");
    }

    if (ret == 0) {
        return;
    }

    for (const auto& pollfd : pollfds) {
        if (pollfd.revents == 0) {
            continue;
        }
        for (auto [pollType, eventType] : AsyncFD::pollToEventType) {
            if (pollfd.revents & pollType) {
                fdMap[pollfd.fd]->eventCb(eventType);
            }
        }
    }
}
