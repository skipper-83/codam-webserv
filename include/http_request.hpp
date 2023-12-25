#pragma once
#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <algorithm>

#include "config.hpp"


class httpRequest {
   private:
	using httpRequestT = std::multimap<std::string, std::string>;
    void _getHttpHeaders(std::istream &fs);
    void _getHttpStartLine(std::istream &fs);
    void _checkHttpHeaders(void);
    void _setVars(void);
    std::string _httpRequestType;
    std::string _httpAdress;
    std::string _httpProtocol;
    httpRequestT _httpHeaders;
    std::string _httpRequestBody;
    size_t _contentLength = 0;
    size_t _bodyLength = 0;
	bool _headerParseComplete = false;
    bool _bodyComplete = false;
    bool _chunkedRequest = false;
	bool _contentSizetSet = false;
	void _popLastNewLine(void);
	const ServerConfig* _server = nullptr;
	int _port = -1;
	size_t _clientMaxBodySize = DEFAULT_CLIENT_BODY_SIZE;

   public:
    using httpRequestListT = std::vector<std::string>;

    httpRequest();
    httpRequest(std::string input);
    explicit httpRequest(std::istream &fs);
    httpRequest(const httpRequest &src);
    ~httpRequest();

    httpRequest &operator=(const httpRequest &rhs);

    std::string getAdress(void) const;
    std::string getRequestType(void) const;
    std::string getProtocol(void) const;
    std::string getHeader(const std::string &key) const;
    std::string getBody(void) const;
    bool bodyComplete(void) const;
	bool headerComplete(void) const;
    httpRequestListT	getHeaderList(std::string const &key) const;
	void printHeaders(std::ostream &os) const;
    void parseHeader(std::istream &fs);
    void parse(std::string const &input);
    void addToBody(std::istream &fs);
	size_t getBodyLength(void) const;
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
		
	};
};

std::ostream &operator<<(std::ostream &os, httpRequest const &t);

#endif