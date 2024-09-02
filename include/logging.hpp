#pragma once

#include <cpplog.hpp>

#define MINIMUM_LOG_LEVEL CPPLog::Level::INFO
#define LOGGING_SCOPES {"main", "parse config", "httpRequest parser", "httpRequest header parser", "httpRequest body parser", "httpResponse", "AsyncPollArray", "client", "fileHandler", "FileHandler", "WebServSession", "cgi", "util"}
// #define LOGGING_SCOPES {"main", "parse config", "WebServSession", "cgi"}
// #define LOGGING_SCOPES	{"main"}


extern CPPLog logOut;
