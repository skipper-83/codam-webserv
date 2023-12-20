#pragma once

#include <functional>
#include <istream>
#include <list>
#include <memory>

#include "FileDescriptor.hpp"

class FDQueue {
   public:
    enum ListenMode : std::uint8_t { NONE = 0, READ = 1, WRITE = 2, EXCEPT = 4, ALL = 7 };

    void push(std::shared_ptr<FileDescriptor> fd, std::function<void(FileDescriptor &, ListenMode)> callback, ListenMode mode = ListenMode::ALL);
    void pop(const FileDescriptor &fd, ListenMode mode = ListenMode::ALL);

    void listen(timeval *timeval = nullptr);

   private:
    struct FDQueueEntry {
        std::shared_ptr<FileDescriptor> fd;
        std::function<void(FileDescriptor &, ListenMode)> callback;
        ListenMode mode;
    };

    std::list<FDQueueEntry> _queue;
};

using FDQueueListenModeType = std::underlying_type<FDQueue::ListenMode>::type;

inline FDQueue::ListenMode operator|(FDQueue::ListenMode a, FDQueue::ListenMode b) {
    return static_cast<FDQueue::ListenMode>(static_cast<FDQueueListenModeType>(a) | static_cast<FDQueueListenModeType>(b));
}

inline FDQueue::ListenMode operator&(FDQueue::ListenMode a, FDQueue::ListenMode b) {
    return static_cast<FDQueue::ListenMode>(static_cast<FDQueueListenModeType>(a) & static_cast<FDQueueListenModeType>(b));
}

inline FDQueue::ListenMode operator~(FDQueue::ListenMode a) {
    return static_cast<FDQueue::ListenMode>(~static_cast<FDQueueListenModeType>(a));
}

inline FDQueue::ListenMode &operator|=(FDQueue::ListenMode &a, FDQueue::ListenMode b) {
    a = a | b;
    return a;
}

inline FDQueue::ListenMode &operator&=(FDQueue::ListenMode &a, FDQueue::ListenMode b) {
    a = a & b;
    return a;
}