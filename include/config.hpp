#pragma once
#ifndef PARSE_CONFIG_HPP
#define PARSE_CONFIG_HPP

#include <chrono>
#include <functional>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "util.hpp"

#define DEFAULT_CLIENT_BODY_SIZE 1000000
#define DEFAULT_PORT 80
#define DEFAULT_MAX_HEADER_SIZE 4096
#define DEFAULT_READ_SIZE 6000000
#define DEFAULT_WRITE_SIZE 6000000
#define DEFAULT_MAX_WRITE_SIZE 32768
#define DEFAULT_TIMEOUT_SECONDS 30
#define DEFAULT_FD_BACKLOG_SIZE 150
#define DEFAULT_LOCAL_FILE_READBUFFER 60000
#define DEFAULT_ALLOWED_METHODS                                                                                                        \
    {WebServUtil::HttpMethod::GET, false}, {WebServUtil::HttpMethod::POST, false}, {WebServUtil::HttpMethod::PUT, false},              \
        {WebServUtil::HttpMethod::DELETE, false}, {WebServUtil::HttpMethod::HEAD, false}, {WebServUtil::HttpMethod::OPTIONS, false}, { \
        WebServUtil::HttpMethod::PATCH, false                                                                                          \
    }
#define DEFAULT_RESPONSE_PROTOCOL "HTTP/1.1"
#define DEFAULT_SERVER_NAME "Jelle en Alberts webserv 1.0"
#define DEFAULT_MIMETYPE "text/plain"
#define DEFAULT_ROOT "./"
#define DEFAULT_MAX_FILENAME_DISPLAY 20
#define SESSION_COOKIE_NAME "webserv_session"
using SubParsers = std::map<std::string, std::function<void(std::istream&)> >;

struct BodySize {
    size_t value = DEFAULT_CLIENT_BODY_SIZE;
    bool defaultValue = true;
};

struct ListenPort {
    uint16_t value;
};

struct AllowedMethods {
    std::map<WebServUtil::HttpMethod, bool> methods = {DEFAULT_ALLOWED_METHODS};
    bool defaultValue = true;
};

struct ServerNames {
    std::vector<std::string> name_vec;
};

struct AutoIndex {
    bool on = false;
    bool defaultValue = true;
};

struct ErrorPage {
    std::vector<int> errorNumbers;
    std::string page;
};

struct Cgi {
    std::vector<std::string> extensions;
    std::string executor;
    AllowedMethods allowed;
};

struct Location {
    std::string ref;
    std::string root;
    AllowedMethods allowed;
    AutoIndex autoIndex;
    std::vector<Cgi> cgis = {};
    BodySize clientMaxBodySize;
    std::vector<std::string> index_vec;
    Cgi const* getCgiFromPath(std::string path) const;
};

class ServerConfig {
   public:
    ServerNames names;
    AllowedMethods allowed;
    std::vector<ListenPort> ports = {};
    std::vector<Location> locations = {};
    std::vector<ErrorPage> errorPages = {};
    std::vector<Cgi> cgis = {};
    AutoIndex autoIndex;
    BodySize clientMaxBodySize;
    int rank;  // deprecated, used for sorting, still used in tests

    std::string getErrorPage(int errorCode) const;


   private:
    void sortLocations(void);

    friend std::istream& operator>>(std::istream& is, ServerConfig& rhs);
};

class MainConfig {
   private:
    std::vector<ServerConfig> _servers;
    AllowedMethods _allowed;
    AutoIndex _autoIndex;
    BodySize clientMaxBodySize;
    std::unordered_map<uint16_t, ServerConfig*> _portsToServers;
    std::map<std::pair<uint16_t, std::string>, ServerConfig*> _portsNamesToServers;
    std::vector<uint16_t> _ports;
    std::string _configPath;
    void _overrideDefaults(void);
    void _setServerNameAndPortArrays(void);

   public:
    const std::chrono::seconds _timeOutDuration = std::chrono::seconds(DEFAULT_TIMEOUT_SECONDS);

    const ServerConfig* getServerFromPort(uint16_t port);
    const ServerConfig* getServerFromPortAndName(uint16_t port, std::string name);
    const ServerConfig* getServer(uint16_t port, std::string name);
    const std::vector<uint16_t>& getPorts(void);
    void setConfigPath(std::string const &path);
    std::string const &getConfigPath() const {return _configPath;}

    friend std::istream& operator>>(std::istream& is, ErrorPage& rhs);
    friend std::istream& operator>>(std::istream& is, AllowedMethods& rhs);
    friend std::istream& operator>>(std::istream& is, AutoIndex& rhs);
    friend std::istream& operator>>(std::istream& is, ServerNames& rhs);
    friend std::istream& operator>>(std::istream& is, Location& rhs);
    friend std::istream& operator>>(std::istream& is, BodySize& rhs);
    friend std::istream& operator>>(std::istream& is, ListenPort& rhs);
    friend std::istream& operator>>(std::istream& is, Cgi& rhs);
    friend std::istream& operator>>(std::istream& is, ServerConfig& rhs);
    friend std::istream& operator>>(std::istream& is, MainConfig& rhs);

    friend std::ostream& operator<<(std::ostream& os, const MainConfig& rhs);
    friend std::ostream& operator<<(std::ostream& os, const ServerConfig& rhs);
    friend std::ostream& operator<<(std::ostream& os, const ErrorPage& rhs);
    friend std::ostream& operator<<(std::ostream& os, const Location& rhs);
    friend std::ostream& operator<<(std::ostream& os, const BodySize& rhs);
    friend std::ostream& operator<<(std::ostream& os, const AutoIndex& rhs);
    friend std::ostream& operator<<(std::ostream& os, const AllowedMethods& rhs);
};

#endif