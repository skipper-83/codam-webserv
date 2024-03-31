#include "client.hpp"
#include "logging.hpp"

static CPPLog::Instance clientLogI = logOut.instance(CPPLog::Level::INFO, "client");
static CPPLog::Instance clientLogW = logOut.instance(CPPLog::Level::WARNING, "client");
static CPPLog::Instance clientLogE = logOut.instance(CPPLog::Level::WARNING, "client");

void Client::_readFromFile() {
    clientLogI << "I have an inputfile" << CPPLog::end;
	if (!_inputFile)
		return;

    if (_inputFile->bad()) {
        clientLogE << "file is bad" << CPPLog::end;
        if (_inputFile->badCode() == EACCES) {
            _inputFile = nullptr;
            _returnHttpErrorToClient(403);
        } else
            _returnHttpErrorToClient(500);
        return;
    }

    if (_inputFile->readBufferFull()) {
        clientLogI << "file buffer full" << CPPLog::end;
        _response.setCode(200);
        if (!_response.isChunked())
            _clientWriteBuffer = _response.getHeadersForChunkedResponse();
        _clientWriteBuffer += _response.transformLineForChunkedResponse(_inputFile->read());
        clientLogI << "file buffer: " << _clientWriteBuffer << "size:" << _clientWriteBuffer.size() << CPPLog::end;
    }

    if (_inputFile->eof()) {
        clientLogI << "file is at eof" << CPPLog::end;
        std::string tempFileBuffer = _inputFile->read();
        _inputFile = nullptr;
        clientLogI << "file buffer: " << tempFileBuffer << "size:" << tempFileBuffer.size() << CPPLog::end;
        if (!_response.isChunked()) {
            _response.setCode(200);
            _response.setFixedSizeBody(tempFileBuffer);
            _clientWriteBuffer = _response.getFixedBodyResponseAsString();
        } else {
            _clientWriteBuffer += _response.transformLineForChunkedResponse(tempFileBuffer);
            _clientWriteBuffer += _response.transformLineForChunkedResponse("");
        }
    }
}