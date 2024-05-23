#include "logging.hpp"

#ifndef MINIMUM_LOG_LEVEL
#define MINIMUM_LOG_LEVEL CPPLog::Level::INFO
#endif

#ifndef LOGGING_SCOPES
#define LOGGING_SCOPES \
    {}
#endif

CPPLog logOut(std::clog, MINIMUM_LOG_LEVEL, LOGGING_SCOPES);
