#pragma once

#include "httpMessage/http_message.hpp"
#include "httpMessage/http_request.hpp"
#include "async/program.hpp"

class cgiMessage : public httpMessage {
	public:
		cgiMessage(std::string const &cgiPath, const httpRequest* request, std::function<void(std::weak_ptr<AsyncFD>)> addAsyncFdToPollArray);
		~cgiMessage() = default;

		bool isHeadersComplete() const { return _headersComplete; }
		bool isBodyComplete() const { return _bodyComplete; }
		int	checkProgramStatus();
		void setRequest(const httpRequest* request) { _request = request; }
		void setStartCgiTime() { _startTime = std::chrono::steady_clock::now(); }
		std::chrono::time_point<std::chrono::steady_clock> getStartTime() const { return _startTime; }
		// void assignBuffer(std::string &buffer) { _writeBuffer = buffer; }
		bool isTooLarge() const { return _tooLarge; }
		std::string getBuffer() const { return _readBuffer; }

	private:
		// std::string _cgiPath;
		// std::string _cgiScript;
		// std::string &_writeBuffer;
		bool _headersComplete = false;
		bool _bodyComplete = false;
		bool _cgiHasHeadersInOutput = false;
		bool _cgiIsRunning = true;
		int _cgiExitCode = 0;
		std::string _absolutePath;
		std::string _readBuffer;
		std::string::size_type _writeCounter = 0;
		std::map<std::string, std::string> _cgiEnv;
		std::shared_ptr<AsyncProgram> _cgi = nullptr;
		const httpRequest* _request = nullptr;
		void _cgiReadCb(AsyncProgram& cgi);
		void _cgiWriteCb(AsyncProgram& cgi);
		void _makeEnvironment();
		std::function<void(std::weak_ptr<AsyncFD>)> _addAsyncFdToPollArray;
		std::chrono::time_point<std::chrono::steady_clock> _startTime = std::chrono::steady_clock::now();
		bool _tooLarge = false;

};