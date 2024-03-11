#pragma once
#ifndef PARSE_CONFIG_HPP
#define PARSE_CONFIG_HPP

#include <functional>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#define DEFAULT_CLIENT_BODY_SIZE 1000000
#define DEFAULT_PORT 80
#define DEFAULT_MAX_HEADER_SIZE 4096
#define DEFAULT_READ_SIZE 1024
#define DEFAULT_WRITE_SIZE 1024
#define DEFAULT_MAX_WRITE_SIZE 32768
#define DEFAULT_TIMEOUT_SECONDS 5
#define DEFAULT_ALLOWED_METHODS                                                                          \
    {"GET", true}, {"POST", true}, {"PUT", true}, {"DELETE", true}, {"HEAD", true}, {"OPTIONS", true}, { \
        "PATCH", true                                                                                    \
    }
#define DEFAULT_RESPONSE_PROTOCOL "HTTP/1.1"
#define DEFAULT_SERVER_NAME "Jelle en Alberts webserv 1.0"

using SubParsers = std::map<std::string, std::function<void(std::istream&)> >;

struct BodySize {
    size_t value = DEFAULT_CLIENT_BODY_SIZE;
    bool defaultValue = true;
};

struct ListenPort {
    uint16_t value;
};

struct AllowedMethods {
    std::map<std::string, bool> methods = {DEFAULT_ALLOWED_METHODS};
    bool defaultValue = true;
};

struct Location {
    std::string ref;
    std::string root;
    std::vector<std::string> index_vec;
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

class ServerConfig {
   public:
    ServerNames names;
    AllowedMethods allowed;
    std::vector<ListenPort> ports;
    std::vector<Location> locations;
    std::vector<ErrorPage> errorPages;
    AutoIndex autoIndex;
    BodySize clientMaxBodySize;
    int rank;

    std::string getErrorPage(int errorCode) const;
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
    void _overrideDefaults(void);
    void _setServerNameAndPortArrays(void);
	// auto _timeOutDuration;

   public:
	const std::chrono::seconds _timeOutDuration = std::chrono::seconds(DEFAULT_TIMEOUT_SECONDS);

    const ServerConfig* getServerFromPort(int port);
    const ServerConfig* getServerFromPortAndName(int port, std::string name);
    const ServerConfig* getServer(int port, std::string name);
    const std::vector<uint16_t>& getPorts(void);

    friend std::istream& operator>>(std::istream& is, ErrorPage& rhs);
    friend std::istream& operator>>(std::istream& is, AllowedMethods& rhs);
    friend std::istream& operator>>(std::istream& is, AutoIndex& rhs);
    friend std::istream& operator>>(std::istream& is, ServerNames& rhs);
    friend std::istream& operator>>(std::istream& is, Location& rhs);
    friend std::istream& operator>>(std::istream& is, BodySize& rhs);
    friend std::istream& operator>>(std::istream& is, ListenPort& rhs);
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