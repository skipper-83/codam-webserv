#pragma once

#include "config.hpp"
#include "http_message.hpp"
#include "util.hpp"

class httpResponse : public httpMessage {
   private:
    int _responseCode;
    std::string _responseCodeDescription;

   public:
    httpResponse();

    void setCode(int code);
    void setBody(std::string body);
	void setHeader(std::string key, std::string value);
	std::string getRequestAsString(void);
};