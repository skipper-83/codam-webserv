#pragma once

#include "config.hpp"
#include "http_message.hpp"
#include "http_request.hpp"
#include "util.hpp"

extern MainConfig mainConfig;

class httpResponse : public httpMessage {
   private:
    int _responseCode = -1;
	bool _bodyComplete = false;
    std::string _responseCodeDescription;
	const httpRequest* _precedingRequest;
	bool _chunked = false;
	void _setBody(std::string body);
	void _setErrorBody(std::string message = "");
	bool _sessionSet = false;

   public:
    httpResponse();
	httpResponse(httpRequest* const callingRequest);
	httpResponse& operator=(const httpResponse& rhs);

    void setCode(int code, std::string description = "");
	int getCode(void) const { return _responseCode; };
    void setFixedSizeBody(std::string body);
	void setPrecedingRequest(httpRequest* const callingRequest);
	void extractHeaders(const httpMessage* message);
	std::string getStartLine(void) const;
	std::string getFixedBodyResponseAsString(void);
	std::string getHeadersForChunkedResponse(void);
	std::string transformLineForChunkedResponse(std::string line);
	bool isBodyComplete(void) const;
	bool isChunked(void) const;
	void clear();
};