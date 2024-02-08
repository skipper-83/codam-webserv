#include "http_request.hpp"
#include "logging.hpp"

static CPPLog::Instance infoLog = logOut.instance(CPPLog::Level::INFO, "httpRequest parser");
static CPPLog::Instance warningLog = logOut.instance(CPPLog::Level::WARNING, "httpRequest parser");

bool httpRequest::_hasNewLine(std::string &str)
{
	infoLog << "finding newline: " << str.find("\r\n\r\n") << " : " << std::string::npos;
	if (str.find("\r\n\r\n") != std::string::npos)
		return true;
	if (str.find("\n\n") != std::string::npos)
		return true;
	return false;
}

/**
 * @brief Returns the remaining amount of bytes from the current pos of the read pointer
 * 		untill the end. Resets the readpointer to its current position after this operation.
 *
 * @param fs
 * @return std::streampos
 */
std::streampos httpRequest::_remainingLength(std::istream &fs) {
    std::streampos curPos, endPos;

    curPos = fs.tellg();
    fs.seekg(0, std::ios::end);
    endPos = fs.tellg();
    fs.seekg(curPos);
    return (endPos - curPos);
}

/**
 * @brief Helper function. Reads the amount of bytes set in [amountOfBytes] and returns them as a string.
 * Throws expection if the filestream is shorter than the amount of bytes asked for.
 *
 * @param fs
 * @param amountOfBytes
 * @return std::string
 */
std::string httpRequest::_readNumberOfBytesFromFileStream(std::istream &fs, size_t amountOfBytes) {
    std::vector<char> buffer(amountOfBytes);

    if (!fs.read(buffer.data(), amountOfBytes)) {
        throw(std::runtime_error("Failed to read chunk from http request"));
    }
    std::string ret(buffer.begin(), buffer.end());
    return (ret);
}

std::string httpRequest::_getLineWithCRLF(std::istream & is)
{
	std::string line;

	std::getline(is, line);
	if (!line.empty() && line.back() == '\r')
		line.pop_back();
	return line;
}

bool httpRequest::_endsWithNewLine(std::string &str)
{
	(void)str;
	return true;
}