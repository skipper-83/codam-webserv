#pragma once
#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

// #include <fstream>
#include <iostream>
#include <map>
#include <regex>
// #include <sstream>
// #include <algorithm>
#include "http_message.hpp"
#include "config.hpp"
#include "util.hpp"

class httpRequest : public httpMessage{
   private:
    void _getHttpHeaders(std::istream &fs);
    void _getHttpStartLine(std::istream &fs);
    void _checkHttpHeaders(void);
    void _setVars(void);
	void _addToFixedContentSize(std::istream &fs);
	void _addChunkedContent(std::istream &fs);
	void _addUntilNewline(std::istream &fs);
	std::string _httpAdress;
    std::string _httpRequestType;
    size_t _contentLength = 0;
	bool _headerParseComplete = false;
    bool _bodyComplete = false;
    bool _chunkedRequest = false;
	bool _contentSizeSet = false;
	const ServerConfig* _server = nullptr;
	int _port = -1;
	size_t _clientMaxBodySize = DEFAULT_CLIENT_BODY_SIZE;

   public:
    httpRequest();
    httpRequest(std::string input);
    explicit httpRequest(std::istream &fs);
    httpRequest(const httpRequest &src);
    ~httpRequest();

    httpRequest &operator=(const httpRequest &rhs);

    std::string getAdress(void) const;
	std::string getRequestType(void) const;
    bool bodyComplete(void) const;
	bool headerComplete(void) const;
    void parseHeader(std::istream &fs);
    void parse(std::string &input);
    void addToBody(std::istream &fs);
	void setServer(MainConfig &config, int port);

	class httpRequestException : public std::exception
	{
		private:
			const std::string _msg;
			const int _errorNo;
		public:
			httpRequestException(int errorNo, std::string msg) :  _msg(msg), _errorNo(errorNo) {};
			virtual const char* what(void) const noexcept override{
				return _msg.c_str();
			}
			int errorNo(void) const noexcept{
				return _errorNo;
			}
			std::string codeDescription() const noexcept
			{
				return WebServUtil::codeDescription(_errorNo);
			}
		
	};
};

std::ostream &operator<<(std::ostream &os, httpRequest const &t);

#endif