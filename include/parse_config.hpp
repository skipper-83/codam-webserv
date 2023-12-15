#pragma once
#ifndef PARSE_CONFIG_HPP
# define PARSE_CONFIG_HPP

#include <map>
#include <vector>
#include <string>

#define CLIENT_BODY_SIZE 1000
#define ALLOWED_METHODS 		{"GET", true}, {"POST", true}, {"PUT", true}, {"DELETE", true}, {"HEAD", true}, {"OPTIONS", true}, {"PATCH", true}

struct BodySize {
    int value = CLIENT_BODY_SIZE;
    bool defaultValue = true;
};

struct ListenPort {
    int value;
};


struct AllowedMethods{
	std::map<std::string, bool> methods = {
		ALLOWED_METHODS
	};
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

struct ServerConfig {
    BodySize clientMaxBodySize;
    ServerNames names;
    AutoIndex autoIndex;
	AllowedMethods allowed;
    std::vector<ListenPort> ports;
    std::vector<Location> locations;
};

struct MainConfig {
    std::vector<ServerConfig> servers;
    bool autoIndex;
    std::vector<std::string> serverNames;
    BodySize clientMaxBodySize;
};

std::istream& operator>>(std::istream& is, AllowedMethods& lhs);
std::istream& operator>>(std::istream& is, AutoIndex& lhs);
std::istream& operator>>(std::istream& is, ServerNames& lhs);
std::istream& operator>>(std::istream& is, Location& lhs);
std::istream& operator>>(std::istream& is, BodySize& lhs);
std::istream& operator>>(std::istream& is, ListenPort& lhs);
std::istream& operator>>(std::istream& is, ServerConfig& lhs);
std::istream& operator>>(std::istream& is, MainConfig& lhs);

#endif