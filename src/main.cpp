#include <iostream>

#include "hello-world.hpp"

#include "logging.hpp"

#include <vector>

int main() {

    CPPLogStream log = logOut.log(CPPLog::INFO, "main");
    log << "Running hello_world()" << CPPLog::end;
    std::string result = hello_world();
    log << "hello_world() returned " << result << CPPLog::end;
    return 0;
}
