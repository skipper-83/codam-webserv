#pragma once

#include <ostream>
#include <sstream>
#include <string>
#include <unordered_map>

class CPPLog {
   public:
    enum class Level {
        INFO,
        DEBUG,
        WARNING,
        ERROR,
        FATAL,
    };

    using enum Level;

    class Instance;
    class Stream;
    class End;

    static const End end;

    CPPLog(std::ostream &os);

    Stream stream(Level level, const std::string &scope);
    Instance instance(Level level, const std::string &scope);
    void log(Level level, const std::string &scope, const std::string &message);

   private:
    void _log(Level level, const std::string &scope, const std::stringstream &message);
    void _printPrefix(Level level, const std::string &scope);
    std::ostream &_output;

    static const std::unordered_map<CPPLog::Level, const char *> _levelToString;
};

class CPPLog::Instance {
   public:
    Instance(Level level, const std::string &scope, CPPLog &log);
    Instance(const Instance &other) = default;
    Instance &operator=(const Instance &other) = default;

    template <typename T>
    Stream operator<<(const T &value);

    Stream stream();
    void log(const std::string &message);

   private:
    Level _level;
    std::string _scope;
    CPPLog &_log;
};

class CPPLog::Stream {
   public:
    Stream(const Instance &instance);
    Stream(const Stream &other) = delete;
    Stream &operator=(const Stream &other) = delete;
    ~Stream();

    template <typename T>
    Stream &operator<<(const T &value) {
        _buffer << value;
        return *this;
    }

    Stream &operator<<(const End &end);

   private:
    template <typename T>
    Stream(const Instance &instance, const T &value);
    Instance _instance;
    std::stringstream _buffer;
    friend class Instance;
};

class CPPLog::End {};

template <typename T>
CPPLog::Stream CPPLog::Instance::operator<<(const T &value) {
    return (Stream(*this, value));
}

template <typename T>
CPPLog::Stream::Stream(const Instance &instance, const T &value) : _instance(instance) {
    _buffer << value;
}