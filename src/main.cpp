#include <functional>
#include <iostream>
#include <vector>

#include "hello-world.hpp"
#include "logging.hpp"

CPPLog::Instance mainLogI = logOut.instance(CPPLog::INFO, "main");

int main() {
    mainLogI << "main() called" << CPPLog::end;
    mainLogI << "hello_world() returned: " << hello_world() << CPPLog::end;
}
