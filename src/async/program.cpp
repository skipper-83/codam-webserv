#include "async/program.hpp"

#include <cstring>
#include <unistd.h>
#include "logging.hpp"

static CPPLog::Instance logD = logOut.instance(CPPLog::Level::DEBUG, "Async");
static CPPLog::Instance logI = logOut.instance(CPPLog::Level::INFO, "Async");
static CPPLog::Instance logW = logOut.instance(CPPLog::Level::WARNING, "Async");
static CPPLog::Instance logE = logOut.instance(CPPLog::Level::ERROR, "Async");
static CPPLog::Instance logF = logOut.instance(CPPLog::Level::FATAL, "Async");

class Environment {
   public:
    Environment();
    Environment(const Environment &other);
    Environment(const std::map<std::string, std::string> &environment);
    ~Environment();

    Environment &operator=(const Environment &other);

    operator char **();

   private:
    void _destroy();
    char **_env;
};

Environment::Environment() : _env(nullptr) {}
Environment::Environment(const Environment &other) {
    operator=(other);
}
Environment::Environment(const std::map<std::string, std::string> &environment) : _env(nullptr) {
    try {
        _env = new char*[environment.size() + 1];
		std::fill(_env, _env + environment.size() + 1, nullptr);

        size_t i = 0;
        for (auto &[key, value] : environment) {
            std::string keyvalue = key + '=' + value;
            _env[i] = new char[keyvalue.length()];
            std::strcpy(_env[i], keyvalue.c_str());
        }
    } catch (std::bad_alloc &e) {
        logE << "Environment::Environment(const std::map<std::string, std::string>&) failed: " << e.what();
        _destroy();
        throw std::bad_alloc();
    }
}

Environment::~Environment() {
    _destroy();
}

Environment &Environment::operator=(const Environment &other) {
    if (this == &other)
        return (*this);

    _destroy();
    if (other._env == nullptr) {
        this->_env = nullptr;
        return (*this);
    }
    size_t length = 0;
    while (other._env[length++] != nullptr)
        ;

    this->_env = new char *[length];
	std::fill(_env, _env + length, nullptr);
    for (size_t i = 0; other._env[i] != nullptr; i++) {
        this->_env[i] = new char[strlen(other._env[i])];
        strcpy(this->_env[i], other._env[i]);
    }
    return (*this);
}

Environment::operator char **() {
    return (_env);
}

void Environment::_destroy() {
    if (_env != nullptr) {
        for (size_t i = 0; _env[i] != nullptr; i++) {
            delete _env[i];
            _env[i] = nullptr;
        }
    }
    delete [] _env;
    _env = nullptr;
}

static void _closePipeFds(int *pipeFds) {
    if (pipeFds[0] != -1) {
        if (close(pipeFds[0]) == -1) {
            logE << "_closePipeFds(int*) failed: close() failed: " << std::strerror(errno);
        }
    }
    if (pipeFds[1] != -1) {
        if (close(pipeFds[1]) == -1) {
            logE << "_closePipeFds(int*) failed: close() failed: " << std::strerror(errno);
        }
    }
}

AsyncProgram::AsyncProgram(const std::string &exec, const std::string &file, const std::map<std::string, std::string> &environment, const ProgramCallback &programReadReadyCb,
                           const ProgramCallback &programWriteReadyCb)
    : _pipeReadFD(nullptr), _pipeWriteFD(nullptr), _programReadReadyCb(programReadReadyCb), _programWriteReadyCb(programWriteReadyCb) {
    logD << "AsyncProgram::AsyncProgram(const std::string&, const std::map<std::string, std::string>&, const ProgramCallback&, const "
            "ProgramCallback&) called";
    Environment env(environment);
    int pipeRead[2] = {-1, -1};
    int pipeWrite[2] = {-1, -1};

    if (pipe(pipeRead) == -1) {
        _closePipeFds(pipeRead);
        _closePipeFds(pipeWrite);
        logE << "AsyncProgram::AsyncProgram(const std::string&, const std::map<std::string, std::string>&, const ProgramCallback&, const "
                "ProgramCallback&) failed: pipe() failed: "
             << std::strerror(errno);
        throw std::runtime_error(std::strerror(errno));
    }
    if (pipe(pipeWrite) == -1) {
        _closePipeFds(pipeRead);
        _closePipeFds(pipeWrite);
        logE << "AsyncProgram::AsyncProgram(const std::string&, const std::map<std::string, std::string>&, const ProgramCallback&, const "
                "ProgramCallback&) failed: pipe() failed: "
             << std::strerror(errno);
        throw std::runtime_error(std::strerror(errno));
    }

    pid_t pid = fork();
    if (pid == -1) {    // if fork() fails
        _closePipeFds(pipeRead);
        _closePipeFds(pipeWrite);
        logE << "AsyncProgram::AsyncProgram(const std::string&, const std::map<std::string, std::string>&, const ProgramCallback&, const "
                "ProgramCallback&) failed: fork() failed: "
             << std::strerror(errno);
        throw std::runtime_error(std::strerror(errno));
    }

    if (pid == 0) { // Child
        if (dup2(pipeRead[1], STDOUT_FILENO) == -1) {
            logF << "AsyncProgram::AsyncProgram(const std::string&, const std::map<std::string, std::string>&, const ProgramCallback&, const "
                    "ProgramCallback&) failed: dup2() failed: "
                 << std::strerror(errno);
            exit(EXIT_FAILURE);
        }
        if (dup2(pipeWrite[0], STDIN_FILENO) == -1) {
            logF << "AsyncProgram::AsyncProgram(const std::string&, const std::map<std::string, std::string>&, const ProgramCallback&, const "
                    "ProgramCallback&) failed: dup2() failed: "
                 << std::strerror(errno);
            exit(EXIT_FAILURE);
        }
        _closePipeFds(pipeRead);
        _closePipeFds(pipeWrite);

        char *argv[3];
        try {
            argv[0] = new char[exec.length() + 1];
            argv[1] = new char[file.length() + 1];
            argv[2] = nullptr;
        } catch (std::bad_alloc &e) {
            logF << "AsyncProgram::AsyncProgram(const std::string&, const std::map<std::string, std::string>&, const ProgramCallback&, const "
                    "ProgramCallback&) failed: " << e.what();
            exit(EXIT_FAILURE);
        }
        std::strcpy(argv[0], exec.c_str());
        std::strcpy(argv[1], file.c_str());

        execvpe(exec.c_str(), argv, env);

        logF << "AsyncProgram::AsyncProgram(const std::string&, const std::map<std::string, std::string>&, const ProgramCallback&, const "
                "ProgramCallback&) failed: execve() failed: " << std::strerror(errno);

        exit(EXIT_FAILURE);  // if execve() fails, we should exit
    }
    // Parent
    if (close(pipeRead[1]) < 0 || close(pipeWrite[0]) < 0) {
        logW << "AsyncProgram::AsyncProgram(const std::string&, const std::map<std::string, std::string>&, const ProgramCallback&, const "
                "ProgramCallback&) failed: close() failed: "
             << std::strerror(errno);
    }
    _pipeReadFD = AsyncInput::create(pipeRead[0], {{AsyncInput::EventTypes::IN, std::bind(&AsyncProgram::_internalReadReadyCb, this, std::placeholders::_1)}});
    _pipeWriteFD = AsyncOutput::create(pipeWrite[1], {{AsyncOutput::EventTypes::OUT, std::bind(&AsyncProgram::_internalWriteReadyCb, this, std::placeholders::_1)}});
}

std::unique_ptr<AsyncProgram> AsyncProgram::create(const std::string &exec, const std::string &file, const std::map<std::string, std::string> &environment,
                                                   const ProgramCallback &programReadReadyCb, const ProgramCallback &programWriteReadyCb) {
    logD << "AsyncProgram::create(const std::string&, const std::map<std::string, std::string>&, const ProgramCallback&, const ProgramCallback&) "
            "called";
    return std::make_unique<AsyncProgram>(exec, file, environment, programReadReadyCb, programWriteReadyCb);
}

AsyncProgram::~AsyncProgram() {
    logD << "AsyncProgram::~AsyncProgram() called";
}

std::string AsyncProgram::read(size_t size) {
	logD << "AsyncProgram::read(size_t) called";

	return _pipeReadFD->read(size);
}

size_t AsyncProgram::write(std::string &data) {
	logD << "AsyncProgram::write(const std::string&) called";

	return _pipeWriteFD->write(data);
}

bool AsyncProgram::hasPendingRead() {
	logD << "AsyncProgram::hasPendingRead() called";

	return _pipeReadFD->hasPendingRead();
}

bool AsyncProgram::hasPendingWrite() {
	logD << "AsyncProgram::hasPendingWrite() called";

	return _pipeWriteFD->hasPendingWrite();
}	

bool AsyncProgram::eof() {
	logD << "AsyncProgram::eof() called";

	return _pipeReadFD->eof();
}

bool AsyncProgram::isReadFdValid() {
	logD << "AsyncProgram::isReadFdValid() called";

	return _pipeReadFD->isValid();
}

bool AsyncProgram::isWriteFdValid() {
	logD << "AsyncProgram::isWriteFdValid() called";

	return _pipeWriteFD->isValid();
}

void AsyncProgram::closeInputFd() {
	logD << "AsyncProgram::closeInputFd() called";

	_pipeReadFD->close();
}

void AsyncProgram::closeOutputFd() {
	logD << "AsyncProgram::closeOutputFd() called";

	_pipeWriteFD->close();
}

void AsyncProgram::_internalReadReadyCb(AsyncFD &) {
	logD << "AsyncProgram::_internalReadReadyCb(AsyncFD&) called";

	if (_programReadReadyCb) {
		_programReadReadyCb(*this);
	}
}

void AsyncProgram::_internalWriteReadyCb(AsyncFD &) {
	logD << "AsyncProgram::_internalWriteReadyCb(AsyncFD&) called";

	if (_programWriteReadyCb) {
		_programWriteReadyCb(*this);
	}
}

void AsyncProgram::addToPollArray(const std::function<void(std::shared_ptr<AsyncFD>)> &addCb)
{
    addCb(_pipeReadFD);
    addCb(_pipeWriteFD);
}