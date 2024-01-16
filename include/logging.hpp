#pragma once

#include <cpplog.hpp>
#include <iostream>

#define MINIMUM_LOG_LEVEL CPPLog::Level::INFO
#define LOGGING_SCOPES {"main", "hello-world", "parse config", "httpRequest parser"}

extern CPPLog logOut;
