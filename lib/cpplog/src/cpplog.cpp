#include "cpplog.hpp"

const std::map<CPPLog::Level, const char *> CPPLog::_convertLevel = {
    {INFO, "INFO"}, {DEBUG, "DEBUG"}, {WARNING, "WARNING"}, {ERROR, "ERROR"}, {FATAL, "FATAL"},
};

CPPLog::CPPLog(std::ostream &os) : _os(os) {}

CPPLogStream CPPLog::log(Level level, const std::string &funcName) {
    return CPPLogStream(*this, level, funcName);
}

void CPPLog::_printPrefix(Level level, const std::string &funcName) {
    _os << "[" << _convertLevel.at(level) << "]<" << funcName << ">: ";
}

const CPPLogStreamEnd CPPLog::end = CPPLogStreamEnd();