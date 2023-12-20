#include <fstream>

#include "hello-world.hpp"
#include "logging.hpp"
#include "parse_config.hpp"

CPPLog::Instance mainLogI = logOut.instance(CPPLog::Level::INFO, "main");

int main(int argc, char** argv) {
    mainLogI << "main() called" << CPPLog::end;
    mainLogI << "hello_world() returned: " << hello_world() << CPPLog::end;

    std::fstream file;
    MainConfig config;
    if (argc != 2)
        return 1;

    file.open(argv[1]);
    if (!file)
        return 1;
    try {
        file >> config;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }

    // config.servers[0].clientMaxBodySize.value = 99;
    std::cout << config;
}
