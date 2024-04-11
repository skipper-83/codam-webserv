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
	bool _chunked = false;
	void _setBody(std::string body);
	void _setErrorBody();

   public:
    httpResponse();
	httpResponse(httpRequest* const callingRequest);
	httpResponse& operator=(const httpResponse& rhs);

    void setCode(int code);
    void setFixedSizeBody(std::string body);
	void setPrecedingRequest(httpRequest* const callingRequest);
	std::string getStartLine(void) const;
	std::string getFixedBodyResponseAsString(void);
	std::string getHeadersForChunkedResponse(void);
	std::string transformLineForChunkedResponse(std::string line);
	bool isBodyComplete(void) const;
	bool isChunked(void) const;
	void clear();
};