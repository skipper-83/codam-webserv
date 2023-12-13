#include "cpplog.hpp"

const CPPLog::End CPPLog::end;

CPPLog::CPPLog(std::ostream &os) : _output(os) {}

CPPLog::Stream CPPLog::stream(Level level, const std::string &scope) {
    return (Stream(instance(level, scope)));
}

CPPLog::Instance CPPLog::instance(Level level, const std::string &scope) {
    return (Instance(level, scope, *this));
}

void CPPLog::log(Level level, const std::string &scope, const std::string &message) {
    _printPrefix(level, scope);
    _output << message << std::endl;
}

void CPPLog::_log(Level level, const std::string &scope, const std::stringstream &message) {
    log(level, scope, message.str());
}

void CPPLog::_printPrefix(Level level, const std::string &scope) {
    _output << "[" << _levelToString.at(level) << "]<" << scope << "> ";
}

const std::unordered_map<CPPLog::Level, const char *> CPPLog::_levelToString = {
    {CPPLog::Level::INFO, "INFO"},   {CPPLog::Level::DEBUG, "DEBUG"}, {CPPLog::Level::WARNING, "WARNING"},
    {CPPLog::Level::ERROR, "ERROR"}, {CPPLog::Level::FATAL, "FATAL"},
};

CPPLog::Instance::Instance(Level level, const std::string &scope, CPPLog &log) : _level(level), _scope(scope), _log(log) {}

CPPLog::Stream CPPLog::Instance::stream() {
    return (*this);
}

void CPPLog::Instance::log(const std::string &message) {
    _log.log(_level, _scope, message);
}

CPPLog::Stream::Stream(const Instance &instance) : _instance(instance) {}

CPPLog::Stream::~Stream() {
    if (_buffer.str().empty())
        return;
    _instance.log(_buffer.str());
}

CPPLog::Stream &CPPLog::Stream::operator<<(const End &) {
    _instance.log(_buffer.str());
    _buffer = std::stringstream();
    return (*this);
}
