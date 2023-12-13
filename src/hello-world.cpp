#include "hello-world.hpp"

#include "logging.hpp"

CPPLog::Instance hwLogI = logOut.instance(CPPLog::INFO, "hello-world");

std::string hello_world() {
    hwLogI << "hello_world() called" << CPPLog::end;
    return std::string("Hello, World!");
}
