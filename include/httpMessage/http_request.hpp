#pragma once
#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include "buffer.hpp"
#include "config.hpp"
#include "httpMessage/http_message.hpp"
#include "util.hpp"

extern MainConfig mainConfig;

class httpRequest : public httpMessage {
   private:
    // PARSERS
    void _parseHttpHeaders(Buffer &input);
    void _parseHttpStartLine(Buffer &input);
    void _checkHttpHeaders(void);

    // HELPERS
    void _addToFixedContentSize(Buffer &input);
    void _addChunkedContent(Buffer &input);
    void _addUntilNewline(Buffer &input);
    bool _hasNewLine(std::string &str);
    std::string _readNumberOfBytesFromFileStream(std::istream &fs, size_t amountOfBytes);
    size_t _remainingLength(std::istream &fs);

    // SETTERS AND RESOLVERS
    void _resolvePathAndLocationBlock(void);
    void _setVars(void);

    std::string _httpAdress;
    WebServUtil::HttpMethod _httpMethod;
    std::map<std::string, std::string> _cookies;
    size_t _contentLength = 0;
    bool _headerParseComplete = false;
    bool _bodyComplete = false;
    bool _chunkedRequest = false;
    bool _contentSizeSet = false;
    bool _returnAutoIndex = false;
    bool _pathSet = false;
    bool _methodCheck = false;
    bool _sessionSet = false;
    const ServerConfig *_server = nullptr;
    const Location *_location = nullptr;
    uint16_t _port = -1;
    std::string _path;
    std::string _queryString = "";
    size_t _clientMaxBodySize = DEFAULT_CLIENT_BODY_SIZE;
    size_t _nextChunkSize = 0;
    bool _chunkSizeKnown = false;
    bool _firstNewLineFound = false;

   public:
    // CONSTRUCTORS
    httpRequest();
    httpRequest(const httpRequest &src);
    ~httpRequest();

    httpRequest &operator=(const httpRequest &rhs);

    // GETTERS
    std::string getAdress(void) const;
    WebServUtil::HttpMethod getMethod(void) const;
    std::string getErrorPage(int errorCode) const;
    bool bodyComplete(void) const;
    bool headerComplete(void) const;
    bool returnAutoIndex(void) const;
    const ServerConfig *getServer(void) const;
    uint16_t getPort(void) const;
    const Location *getLocation(void) const;
    std::string getPath(void) const;
    std::map<std::string, std::string> getCookies(void) const;
    std::string getCookie(std::string key) const;
    bool isSessionSet(void) const { return _sessionSet; }
    std::string getQueryString(void) const { return _queryString; }

    // PARSERS
    void parseHeader(Buffer &input);  // only called internally and for testing
    void parseBody(Buffer &input);    // only called internally and for testing
    void parse(Buffer &input, uint16_t port);
    void parseCookieHeader(std::string cookieHeader);

    // SETTERS
    void setServer(MainConfig &config, uint16_t port);
    void clear(Buffer &buffer);  // clears the request
    void setSession(bool session) { _sessionSet = session; }

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