#pragma once

#include <cpplog.hpp>
#include <iostream>

#define MINIMUM_LOG_LEVEL CPPLog::Level::INFO
#define LOGGING_SCOPES {"main", "parse config", "httpRequest parser", "httpRequest header parser", "httpRequest body parser", "httpResponse", "AsyncPollArray", "client", "fileHandler", "FileHandler", "WebServSession"}

extern CPPLog logOut;
