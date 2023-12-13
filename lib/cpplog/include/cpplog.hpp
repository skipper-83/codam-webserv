#pragma once

#include <map>
#include <ostream>
#include <string>

class CPPLogStream;
class CPPLogStreamEnd {};

class CPPLog {
   public:
    enum Level {
        INFO,
        DEBUG,
        WARNING,
        ERROR,
        FATAL,
    };

    CPPLog(std::ostream& os);

    CPPLogStream log(Level level, const std::string& funcName);

    static const CPPLogStreamEnd end;

   protected:
    void _printPrefix(Level level, const std::string& funcName);
    std::ostream& _os;
    bool _IsPrintPrefix = true;

    static const std::map<Level, const char*> _convertLevel;
    friend class CPPLogStream;
};

class CPPLogStream {
   public:
    CPPLogStream(CPPLog& log, CPPLog::Level level, const std::string& funcName) : _log(log), _level(level), _funcName(funcName) {}

    inline CPPLogStream& operator<<(const CPPLogStreamEnd &) {
        _log._os << std::endl;
        _log._IsPrintPrefix = true;
        return *this;
    }

    template <typename T>
    CPPLogStream& operator<<(const T& msg) {
        if (_log._IsPrintPrefix) {
            _log._printPrefix(_level, _funcName);
            _log._IsPrintPrefix = false;
        }
        _log._os << msg;
        return *this;
    }

   private:
    CPPLog& _log;
    CPPLog::Level _level;
    std::string _funcName;
};
