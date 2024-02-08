#pragma once

// #include <fstream>
#include <iostream>
#include <map>
// #include <regex>
// #include <sstream>
// #include <algorithm>
// #include "config.hpp"

class WebServUtil
{
	private:
	static const std::map<int, std::string>& getStatusCodes();

	
	public:
	static std::string codeDescription(int httpCode);
	static std::string timeStamp(void);
	static bool isRequestWithoutBody(std::string requestType);
};