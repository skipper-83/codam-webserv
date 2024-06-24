#include "cpplog.hpp"

const CPPLog::End CPPLog::end;

CPPLog::CPPLog(std::ostream &output, Level minLevel, const std::unordered_set<std::string> &scopes)
    : _output(output), _minLevel(minLevel), _scopes(scopes) {}
CPPLog::Stream CPPLog::stream(Level level, const std::string &scope) {
    return (Stream(instance(level, scope)));
}

CPPLog::Instance CPPLog::instance(Level level, const std::string &scope) {
    return (Instance(level, scope, *this));
}

void CPPLog::log(Level level, const std::string &scope, const std::string &message) {
    if (level < _minLevel)
        return;
    if (!_scopes.empty() && _scopes.find(scope) == _scopes.end())
        return;

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
    {CPPLog::Level::ERROR, "ERROR"}, {CPPLog::Level::FATAL, "FATAL"}, {CPPLog::Level::NONE, "NONE"},
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
