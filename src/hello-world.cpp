#include "hello-world.hpp"

#include "logging.hpp"

std::string hello_world() {
    logOut.log(CPPLog::INFO, "hello_world") << "return Hello, World!" << CPPLog::end;
    return std::string("Hello, World!");
}
