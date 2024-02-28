#pragma once

#include <cpplog.hpp>
#include <iostream>

#define MINIMUM_LOG_LEVEL CPPLog::Level::INFO

#define LOGGING_SCOPES {"main", "hello-world", "parse config", "httpRequest parser", "httpRequest header parser", "httpRequest body parser", "httpResponse","client"}

extern CPPLog logOut;
