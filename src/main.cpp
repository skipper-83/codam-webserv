#include "logging.hpp"
#include "server.hpp"

CPPLog::Instance mainLogI = logOut.instance(CPPLog::INFO, "main");

int main() {
    mainLogI << "Starting server" << CPPLog::end;
    server(8080);

    return 0;
}
