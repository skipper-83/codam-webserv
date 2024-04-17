#pragma once
#include "input.hpp"
#include "output.hpp"

class AsyncProgram {
   public:
    using ProgramCallback = std::function<void(AsyncProgram &)>;

    AsyncProgram(const std::string &exec, const std::string &file, const std::map<std::string, std::string> &environment, const ProgramCallback &programReadReadyCb = {},
                 const ProgramCallback &programWriteReadyCb = {});
    static std::unique_ptr<AsyncProgram> create(const std::string &exec, const std::string &file, const std::map<std::string, std::string> &environment,
                                                const ProgramCallback &programReadReadyCb = {}, const ProgramCallback &programWriteReadyCb = {});

    virtual ~AsyncProgram();

    AsyncProgram(const AsyncProgram &) = delete;
    AsyncProgram &operator=(const AsyncProgram &) = delete;

    AsyncProgram(const AsyncProgram &&) = delete;
    AsyncProgram &operator=(const AsyncProgram &&) = delete;

    void addToPollArray(const std::function<void(std::weak_ptr<AsyncFD>)> &addCb);

	std::string read(size_t size);
	size_t write(std::string &data);

	bool hasPendingRead();
	bool hasPendingWrite();

	bool eof();

	bool isReadFdValid();
	bool isWriteFdValid();

	void closeInputFd();
	void closeOutputFd();

   protected:
    void _internalReadReadyCb(AsyncFD &fd);
	void _internalWriteReadyCb(AsyncFD &fd);

    std::shared_ptr<AsyncInput> _pipeReadFD;
    std::shared_ptr<AsyncOutput> _pipeWriteFD;

    ProgramCallback _programReadReadyCb;
    ProgramCallback _programWriteReadyCb;
};