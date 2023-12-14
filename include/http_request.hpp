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
    std::string _httpRequestType;
    std::string _httpAdress;
    std::string _httpProtocol;
    httpRequestT _httpHeaders;
    std::string _httpRequestBody;

   public:
    using httpRequestListT = std::vector<std::string>;

    httpRequest();
    httpRequest(std::string input);
    httpRequest(std::istream &fs);
    httpRequest(const httpRequest &src);
    ~httpRequest();

    httpRequest &operator=(const httpRequest &rhs);

    std::string getAdress(void) const;
    std::string getRequestType(void) const;
    std::string getProtocol(void) const;
    std::string getHeader(const std::string &key) const;
    std::string getBody(void) const;
    httpRequestListT	getHeaderList(std::string const &key) const;
	void printHeaders(std::ostream &os) const;
    void parse(std::istream &fs);
    void parse(std::string const &input);
};

std::ostream &operator<<(std::ostream &os, httpRequest const &t);

#endif