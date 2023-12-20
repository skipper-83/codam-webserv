#include "FDQueue.hpp"

#include <algorithm>
#include <cstring>
#include <stdexcept>

using namespace std;

void FDQueue::push(std::shared_ptr<FileDescriptor> fd, std::function<void(FileDescriptor &, ListenMode)> callback, ListenMode mode) {
    if (mode == ListenMode::NONE) {
        return;
    }
    _queue.push_back({fd, callback, mode});
}

void FDQueue::pop(const FileDescriptor &fd, ListenMode mode) {
    _queue.remove_if([&](FDQueueEntry &entry) {
        if (*entry.fd != fd)
            return false;
        entry.mode &= ~mode;
        return entry.mode == ListenMode::NONE;
    });
}

void FDQueue::listen(timeval *timeval) {
    fd_set read_fds;
    fd_set write_fds;
    fd_set except_fds;

    int max_fd = 0;

    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&except_fds);

    for (auto &entry : _queue) {
        if ((entry.mode & ListenMode::READ)) {
            FD_SET(entry.fd->getFD(), &read_fds);
        }
        if (entry.mode & ListenMode::WRITE) {
            FD_SET(entry.fd->getFD(), &write_fds);
        }
        if (entry.mode & ListenMode::EXCEPT) {
            FD_SET(entry.fd->getFD(), &except_fds);
        }
        max_fd = std::max(max_fd, entry.fd->getFD());
    }
    int result = select(max_fd + 1, &read_fds, &write_fds, &except_fds, timeval);
    if (result < 0) {
        throw std::runtime_error(std::string("Select failed, errno: ") + std::strerror(errno));
    }

    for (auto it = _queue.begin(); it != _queue.end();) {
        auto &entry = *it;
        it++;
        if (FD_ISSET(entry.fd->getFD(), &read_fds)) {
            entry.callback(*entry.fd, ListenMode::READ);
        }
    }
    for (auto it = _queue.begin(); it != _queue.end();) {
        auto &entry = *it;
        it++;
        if (FD_ISSET(entry.fd->getFD(), &write_fds)) {
            entry.callback(*entry.fd, ListenMode::WRITE);
        }
    }
    for (auto it = _queue.begin(); it != _queue.end();) {
        auto &entry = *it;
        it++;
        if (FD_ISSET(entry.fd->getFD(), &except_fds)) {
            entry.callback(*entry.fd, ListenMode::EXCEPT);
        }
    }
}
