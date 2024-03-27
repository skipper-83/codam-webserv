#pragma once

#include <filesystem>
#include <iostream>
#include <map>
#include <unordered_map>

class WebServUtil {
   private:
    static const std::map<int, std::string> &_getStatusCodes();
    static const std::unordered_map<std::string, std::string> _getFileTypes(void);
    static bool _compareDirectoryContents(const std::filesystem::path &one, const std::filesystem::path &two);

   public:
    static std::string codeDescription(int httpCode);
    static std::string getContentTypeFromPath(const std::string &path);
    static std::string timeStamp(void);
    static bool isRequestWithoutBody(std::string requestType);
    static std::string directoryIndexList(const std::string &path, const std::string &request_adress);
};