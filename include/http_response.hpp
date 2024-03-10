#pragma once

#include "config.hpp"
#include "http_message.hpp"
#include "http_request.hpp"
#include "util.hpp"

extern MainConfig mainConfig;

class httpResponse : public httpMessage {
   private:
    int _responseCode;
	bool _bodyComplete = false;
    std::string _responseCodeDescription;
	const httpRequest* _precedingRequest;
	void setErrorBody();

   public:
    httpResponse();
	httpResponse(httpRequest* const callingRequest);
	httpResponse& operator=(const httpResponse& rhs);

    void setCode(int code);
    void setFixedSizeBody(std::string body);
	void setPrecedingRequest(httpRequest* const callingRequest);
	std::string getFixedBodyResponseAsString(void);
	bool isBodyComplete(void) const;
	void clear();
};