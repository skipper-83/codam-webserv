#pragma once
#include "fd.hpp"

class AsyncOutput : virtual public AsyncFD {
   public:
	AsyncOutput(int fd, const std::map<EventTypes, EventCallback>& eventCallbacks = {});
	AsyncOutput(const std::map<EventTypes, EventCallback>& eventCallbacks = {});
	static std::unique_ptr<AsyncOutput> create(int fd, const std::map<EventTypes, EventCallback>& eventCallbacks = {});
	virtual ~AsyncOutput();

	AsyncOutput(const AsyncOutput&) = delete;
	AsyncOutput& operator=(const AsyncOutput&) = delete;

	AsyncOutput(AsyncOutput&&) = delete;
	AsyncOutput& operator=(AsyncOutput&&) = delete;

	size_t write(std::string & data);
	size_t write(const char* data, ssize_t length = -1);
	bool hasPendingWrite() const;

   protected:
	static void _internalOutCb(AsyncFD& fd);
	EventCallback _outCb;
	bool _hasPendingWrite;
};
