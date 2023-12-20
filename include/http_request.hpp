#pragma once
#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

// # include <map>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <algorithm>

class httpRequest {
   private:
    using httpRequestT = std::multimap<std::string, std::string>;

    void _getHttpHeaders(std::istream &fs);
    void _getHttpStartLine(std::istream &fs);
    void _checkHttpHeaders(void);
    void _parseRequestBody(std::istream &fs);
    void _setVars(void);
    std::string _httpRequestType;
    std::string _httpAdress;
    std::string _httpProtocol;
    httpRequestT _httpHeaders;
    std::string _httpRequestBody;
    int _contentLength = 0;
    int _bodyLength = 0;
    bool _requestDone = false;
    bool _chunkedRequest = false;

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
    bool isDone(void) const;
    httpRequestListT	getHeaderList(std::string const &key) const;
	void printHeaders(std::ostream &os) const;
    void parse(std::istream &fs);
    void parse(std::string const &input);
    void addToBody(std::istream &fs);
};

std::ostream &operator<<(std::ostream &os, httpRequest const &t);

#endif