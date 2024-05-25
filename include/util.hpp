#pragma once

#include <filesystem>
#include <map>
#include <unordered_map>

class WebServUtil {
   private:
    static const std::map<int, std::string> &_getStatusCodes();
    static const std::unordered_map<std::string, std::string> _getFileTypes(void);
    static bool _compareDirectoryContents(const std::filesystem::path &one, const std::filesystem::path &two);
	static	std::string _fileTimeToString(const std::filesystem::file_time_type& fileTime);

   public:
   	enum class HttpMethod {
		GET,
		POST,
		HEAD,
		PUT,
		DELETE,
		CONNECT,
		OPTIONS,
		TRACE,
		PATCH,
		UNKNOWN
	};
    static std::string codeDescription(int httpCode);
    static std::string getContentTypeFromPath(const std::string &path);
    static std::string timeStamp(void);
    static bool isRequestWithoutBody(HttpMethod requestType);
    static std::string directoryIndexList(const std::string &path, const std::string &request_adress);
	static HttpMethod stringToHttpMethod(std::string method);
	static std::string httpMethodToString(HttpMethod method);
};