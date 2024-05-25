#pragma once

#include <iostream>
#include <map>
#include <vector>

#include "config.hpp"

class httpMessage {
   public:
    using httpRequestListT = std::vector<std::string>;
    using httpRequestT = std::multimap<std::string, std::string>;

    virtual ~httpMessage();

    std::string getProtocol(void) const;
    std::string getHeader(const std::string &key) const;
    std::string getBody(void) const;
    httpRequestListT getHeaderList(std::string const &key) const;
	httpRequestT getHeaderMap(void) const;
    std::string getHeaderListAsString(void) const;
    size_t getBodyLength(void) const;
    void deleteHeader(std::string key);
    void setHeader(std::string key, std::string value);

   protected:
    httpMessage(void);

    // httpMessage &operator=(const httpMessage &rhs);

    void _httpMessageAssign(httpMessage const &rhs);
    bool _getLineWithCRLF(std::istream &fs, std::string &line);
	std::string _getLineWithCRLF(std::istream &is);
	std::string _getLineWithCRLF(std::string &input);
	std::pair<std::string, std::string> _parseHeaderLine(std::string line);
    std::string _httpProtocol = "";
    httpRequestT _httpHeaders;
    std::string _httpBody = "";
    size_t _bodyLength = 0;

};