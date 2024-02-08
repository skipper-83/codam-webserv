#include "http_request.hpp"
#include "logging.hpp"

static CPPLog::Instance infoLog = logOut.instance(CPPLog::Level::INFO, "httpRequest parser");
static CPPLog::Instance warningLog = logOut.instance(CPPLog::Level::WARNING, "httpRequest parser");

void httpRequest::parse(std::string &input, uint16_t port) {
	infoLog << "Parse function called";
	// NEWLINE TEST 1
	infoLog << "Input: [" << input << "]";
    if (!this->_headerParseComplete && !this->_hasNewLine(input)) // header is not complete yet
	{
		infoLog << "Header incomplete";
        return ;
	}
    std::stringstream is(input);

    if (!this->_headerParseComplete) {
        try {
            parseHeader(is);
        } catch (const httpRequestException &e) {
            // reply with bad request response?
			this->clear();
            warningLog << "HTTP error " << e.errorNo() << ": " << e.codeDescription() << "\n" << e.what();
        }
    }
    if (this->_headerParseComplete && this->_server == nullptr)
        try {
            setServer(mainConfig, port);
        } catch (const std::exception &e) {
            // if(dynamic_cast<httpRequestException>(e))
			this->clear();
            throw(httpRequestException(500, e.what()));
            // else
            // throw(httpR)
        }
    if (this->_headerParseComplete && !this->_bodyComplete) {
        try {
            parseBody(is);	
        } catch (const httpRequestException &e) {
            // reply with bad request response?
			this->clear();
            warningLog << "HTTP error " << e.errorNo() << ": " << e.codeDescription() << "\n" << e.what();
        }
    }
	input = is.str().substr(is.tellg());
    std::cerr << "is length now: [" << _remainingLength(is) << "]\n";
}