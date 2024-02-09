#pragma once
#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include "config.hpp"
#include "http_message.hpp"
#include "util.hpp"

extern MainConfig mainConfig;

class httpRequest : public httpMessage {
   private:
    void _parseHttpHeaders(std::istream &fs);
    void _parseHttpStartLine(std::istream &fs);
    void _checkHttpHeaders(void);
    void _setVars(void);
    void _addToFixedContentSize(std::istream &fs);
    void _addChunkedContent(std::istream &fs);
    void _addUntilNewline(std::istream &fs);
    bool _hasNewLine(std::string &str);
    std::string _getLineWithCRLF(std::istream &is);
    std::string _readNumberOfBytesFromFileStream(std::istream &fs, size_t amountOfBytes);
    std::streampos _remainingLength(std::istream &fs);
    std::string _httpAdress;
    std::string _httpRequestType;
    size_t _contentLength = 0;
    bool _headerParseComplete = false;
    bool _bodyComplete = false;
    bool _chunkedRequest = false;
    bool _contentSizeSet = false;
    const ServerConfig *_server = nullptr;
    int _port = -1;
    size_t _clientMaxBodySize = DEFAULT_CLIENT_BODY_SIZE;

   public:
    httpRequest();
    explicit httpRequest(std::istream &fs);
    httpRequest(const httpRequest &src);
    ~httpRequest();

    httpRequest &operator=(const httpRequest &rhs);

    std::string getAdress(void) const;
    std::string getRequestType(void) const;
    std::string getErrorPage(int errorCode) const;
    bool bodyComplete(void) const;
    bool headerComplete(void) const;
    void parseHeader(std::istream &fs);
    void parse(std::string &input, uint16_t port);
    void setServer(MainConfig &config, uint16_t port);
	const ServerConfig* getServer(void) const;
    void parseBody(std::istream &fs);
    void clear(void);

    class httpRequestException : public std::exception {
       private:
        const std::string _msg;
        const int _errorNo;

       public:
        httpRequestException(int errorNo, std::string msg) : _msg(msg), _errorNo(errorNo){};
        virtual const char *what(void) const noexcept override { return _msg.c_str(); }
        int errorNo(void) const noexcept { return _errorNo; }
        std::string codeDescription() const noexcept { return WebServUtil::codeDescription(_errorNo); }
    };
};

std::ostream &operator<<(std::ostream &os, httpRequest const &t);

#endif