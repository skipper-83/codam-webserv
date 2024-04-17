#include "util.hpp"

#include <ctime>
#include <cstring>
#include <chrono>
// #include <format>
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <unordered_map>

#include "config.hpp"

std::string WebServUtil::codeDescription(int httpCode) {
    std::string ret = "Unknown HTTP Status Code";
    std::map<int, std::string> const& codes = _getStatusCodes();
    auto it = codes.find(httpCode);
    if (it != codes.end())
        return it->second;
    return ret;
}

std::string WebServUtil::getContentTypeFromPath(const std::string& path) {
    const std::unordered_map<std::string, std::string> mimeTypes = _getFileTypes();
    std::string extension = std::filesystem::path(path).extension().string();
    auto it = mimeTypes.find(extension);
    if (it != mimeTypes.end())
        return it->second;
    return (DEFAULT_MIMETYPE);
}

bool WebServUtil::_compareDirectoryContents(const std::filesystem::path& one, const std::filesystem::path& two) {
    bool oneIsDir = std::filesystem::is_directory(one);
    bool twoIsDir = std::filesystem::is_directory(two);

    if (oneIsDir == twoIsDir)
        return one.filename().string() < two.filename().string();
    return oneIsDir > twoIsDir;
}

std::string WebServUtil::_fileTimeToString(const std::filesystem::file_time_type& fileTime) {
    using namespace std::chrono;

    // Extracting the duration since the epoch
    auto duration_since_epoch = fileTime.time_since_epoch();

    // Converting to seconds
    auto seconds_since_epoch = duration_cast<seconds>(duration_since_epoch);

    // Constructing time_point from seconds
    system_clock::time_point tp{seconds_since_epoch};

    // Converting time_point to time_t
    auto time_t_point = system_clock::to_time_t(tp);

    // Converting to tm structure
    std::tm tm;

    localtime_r(&time_t_point, &tm);

    // Formatting time
    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}
std::string WebServUtil::directoryIndexList(const std::string& path, const std::string& request_adress) {
    std::vector<std::filesystem::path> entries;
    std::string title, filename;
    std::stringstream index_listing;


    title = "Index of " + request_adress;
    index_listing << "<html><head><title>" << title << "</title></head><body>\n";
    index_listing << "<h1>" << title << "</h1><hr><pre>\n";
    index_listing << ("<a href=\"../\">../</a>\n");
    for (const auto& it : std::filesystem::directory_iterator(path))
        entries.push_back(it.path());
    std::sort(entries.begin(), entries.end(), _compareDirectoryContents);
    for (const auto& it : entries) {
        filename = it.filename().string();
        if (std::filesystem::is_directory(it))
            filename += "/";
		if (filename.length() > DEFAULT_MAX_FILENAME_DISPLAY)
			filename = filename.substr(0, DEFAULT_MAX_FILENAME_DISPLAY) + "..>";
		index_listing << "<a href=\"" << it.filename().string() << "\">" << std::setw(DEFAULT_MAX_FILENAME_DISPLAY) << std::left << filename + "</a>";
		index_listing << "\t" << _fileTimeToString(std::filesystem::last_write_time(it));
		if (!std::filesystem::is_directory(it))
			index_listing << "\t" << std::filesystem::file_size(it);
		else
			index_listing << "\t" << "-";
		index_listing << "\n";
    }
    index_listing << "</pre><hr></body></html>";
    return index_listing.str();
}

WebServUtil::HttpMethod WebServUtil::stringToHttpMethod(std::string method) {
	static const std::unordered_map<std::string, HttpMethod> methods = {
		{"GET", HttpMethod::GET},
		{"POST", HttpMethod::POST},
		{"HEAD", HttpMethod::HEAD},
		{"PUT", HttpMethod::PUT},
		{"DELETE", HttpMethod::DELETE},
		{"CONNECT", HttpMethod::CONNECT},
		{"OPTIONS", HttpMethod::OPTIONS},
		{"TRACE", HttpMethod::TRACE},
		{"PATCH", HttpMethod::PATCH},
	};

	auto it = methods.find(method);
	if (it != methods.end())
		return it->second;
	return HttpMethod::UNKNOWN;
}

std::string WebServUtil::httpMethodToString(HttpMethod method) {
	static const std::unordered_map<HttpMethod, std::string> methods = {
		{HttpMethod::GET, "GET"},
		{HttpMethod::POST, "POST"},
		{HttpMethod::HEAD, "HEAD"},
		{HttpMethod::PUT, "PUT"},
		{HttpMethod::DELETE, "DELETE"},
		{HttpMethod::CONNECT, "CONNECT"},
		{HttpMethod::OPTIONS, "OPTIONS"},
		{HttpMethod::TRACE, "TRACE"},
		{HttpMethod::PATCH, "PATCH"},
	};
	auto it = methods.find(method);
	if (it != methods.end())
		return it->second;
	return "";
}

std::string WebServUtil::timeStamp(void) {
    std::time_t currentTime = std::time(nullptr);
    std::tm* gmtTime = std::gmtime(&currentTime);
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", gmtTime);
    return buffer;
}

bool WebServUtil::isRequestWithoutBody(HttpMethod requestType) {
	if (requestType == HttpMethod::GET || requestType == HttpMethod::HEAD || requestType == HttpMethod::DELETE || requestType == HttpMethod::OPTIONS || requestType == HttpMethod::TRACE || requestType == HttpMethod::CONNECT)
        return true;
    return false;
}

const std::unordered_map<std::string, std::string> WebServUtil::_getFileTypes(void) {
    const std::unordered_map<std::string, std::string> mimeTypes = {
        {".html", "text/html"},       {".htm", "text/html"},
        {".jpg", "image/jpeg"},       {".jpeg", "image/jpeg"},
        {".png", "image/png"},        {".pdf", "application/pdf"},
        {".txt", "text/plain"},       {".css", "text/css"},
        {".js", "text/javascript"},   {"", "application/octet-stream"},
        {".ico", "image/x-icon"},     {".gif", "image/gif"},
        {".svg", "image/svg+xml"},    {".xml", "application/xml"},
        {".json", "application/json"}, {".php", "application/x-httpd-php"},
		{".zip", "application/zip"},  {".tar", "application/x-tar"},
		{".gz", "application/gzip"},  {".rar", "application/x-rar-compressed"},
		{".7z", "application/x-7z-compressed"}, {".tar.gz", "application/gzip"},		
        // Add more mappings here as needed
    };
    return mimeTypes;
}

const std::map<int, std::string>& WebServUtil::_getStatusCodes() {
    static const std::map<int, std::string> codes = {{100, "Continue"},
                                                     {101, "Switching Protocols"},
                                                     {102, "Processing"},
                                                     {103, "Early Hints"},
                                                     {200, "OK"},
                                                     {201, "Created"},
                                                     {202, "Accepted"},
                                                     {203, "Non-Authoritative Information"},
                                                     {204, "No Content"},
                                                     {205, "Reset Content"},
                                                     {206, "Partial Content"},
                                                     {207, "Multi-Status"},
                                                     {208, "Already Reported"},
                                                     {226, "IM Used"},
                                                     {300, "Multiple Choices"},
                                                     {301, "Moved Permanently"},
                                                     {302, "Found"},
                                                     {303, "See Other"},
                                                     {304, "Not Modified"},
                                                     {305, "Use Proxy"},
                                                     {306, "Switch Proxy"},
                                                     {307, "Temporary Redirect"},
                                                     {308, "Permanent Redirect"},
                                                     {400, "Bad Request"},
                                                     {401, "Unauthorized"},
                                                     {402, "Payment Required"},
                                                     {403, "Forbidden"},
                                                     {404, "Not Found"},
                                                     {405, "Method Not Allowed"},
                                                     {406, "Not Acceptable"},
                                                     {407, "Proxy Authentication Required"},
                                                     {408, "Request Timeout"},
                                                     {409, "Conflict"},
                                                     {410, "Gone"},
                                                     {411, "Length Required"},
                                                     {412, "Precondition Failed"},
                                                     {413, "Payload Too Large"},
                                                     {414, "URI Too Long"},
                                                     {415, "Unsupported Media Type"},
                                                     {416, "Range Not Satisfiable"},
                                                     {417, "Expectation Failed"},
                                                     {418, "I'm a teapot"},  // This one is more of an Easter egg in the standard
                                                     {421, "Misdirected Request"},
                                                     {422, "Unprocessable Entity"},
                                                     {423, "Locked"},
                                                     {424, "Failed Dependency"},
                                                     {425, "Too Early"},
                                                     {426, "Upgrade Required"},
                                                     {428, "Precondition Required"},
                                                     {429, "Too Many Requests"},
                                                     {431, "Request Header Fields Too Large"},
                                                     {451, "Unavailable For Legal Reasons"},
                                                     {500, "Internal Server Error"},
                                                     {501, "Not Implemented"},
                                                     {502, "Bad Gateway"},
                                                     {503, "Service Unavailable"},
                                                     {504, "Gateway Timeout"},
                                                     {505, "HTTP Version Not Supported"},
                                                     {506, "Variant Also Negotiates"},
                                                     {507, "Insufficient Storage"},
                                                     {508, "Loop Detected"},
                                                     {510, "Not Extended"},
                                                     {511, "Network Authentication Required"}};
    return codes;
}
