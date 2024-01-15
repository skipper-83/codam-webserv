#include "async/fd.hpp"
#include "async/pollarray.hpp"
#include "logging.hpp"

CPPLog::Instance mainLogI = logOut.instance(CPPLog::INFO, "main");

int main() {
    mainLogI << "Hello, world!" << CPPLog::end;

    std::shared_ptr<AsyncSocket> socket = AsyncSocket::create(8080);

    AsyncPollArray pollArray;

    std::shared_ptr<AsyncIOFD> fd;

    pollArray.add(socket);

    while (true) {
        pollArray.poll(-1);
        if (socket->hasPendingAccept()) {
            fd = socket->accept();
            pollArray.add(fd);
            fd->writeBuffer += "Hello, world!\n";
            mainLogI << "Accepted connection" << CPPLog::end;
        }
        if (fd->readBuffer.size() > 0) {
            if (fd->readBuffer.back() == '\n')
                fd->readBuffer.pop_back();
            mainLogI << "Received: " << fd->readBuffer << CPPLog::end;
            fd->writeBuffer += "received\n";
            if (fd->readBuffer == "quit") {
                mainLogI << "Closing connection" << CPPLog::end;
                fd->close();
            } else if (fd->readBuffer == "exit") {
                fd->close();
                mainLogI << "Exiting" << CPPLog::end;
                break;
            }
            fd->readBuffer.clear();
        }
        pollArray.cleanup();
    }
    socket->close();
    pollArray.cleanup();

    while (1) {
    }
}
