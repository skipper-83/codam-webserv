#pragma once

#include <cpplog.hpp>
#include <iostream>

#define MINIMUM_LOG_LEVEL CPPLog::Level::INFO
#define LOGGING_SCOPES {"main", "parse config", "httpRequest parser", "httpRequest header parser", "httpRequest body parser", "httpResponse", "Async", "AsyncPollArray", "client", "fileHandler"}

extern CPPLog logOut;
