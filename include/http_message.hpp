#pragma once

#include <iostream>
#include <map>
#include <vector>

class httpMessage
{
	public:
		using httpRequestListT = std::vector<std::string>;
		using httpRequestT = std::multimap<std::string, std::string>;

		virtual ~httpMessage() ;

		std::string getProtocol(void) const;
		std::string getHeader(const std::string &key) const;
		std::string getBody(void) const;
		httpRequestListT	getHeaderList(std::string const &key) const;
		std::string getHeaderListAsString(void) const;
		size_t getBodyLength(void) const;
		void deleteHeader(std::string key);
		void setHeader(std::string key, std::string value);
	
	protected:
		httpMessage(void);
		
		std::string _httpProtocol = "";
		httpRequestT _httpHeaders;
		std::string _httpBody = "";
		size_t _bodyLength = 0;

};