#pragma once

#include "config.hpp"
#include "http_message.hpp"
#include "http_request.hpp"
#include "util.hpp"

extern MainConfig mainConfig;

class httpResponse : public httpMessage {
   private:
    int _responseCode;
    std::string _responseCodeDescription;
	const httpRequest* _precedingRequest;
	void setErrorBody();

   public:
    httpResponse();
	httpResponse(httpRequest* const callingRequest);

    void setCode(int code);
    void setBody(std::string body);
	void setPrecedingRequest(httpRequest* const callingRequest);
	// void setHeader(std::string key, std::string value);
	std::string getResponseAsString(void);
};