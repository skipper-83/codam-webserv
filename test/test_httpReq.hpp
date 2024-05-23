#include <gtest/gtest.h>
#include "httpMessage/http_request.hpp"

#include <iostream>
#include <sstream>

class httpRequestTest : public testing::Test {
   private:
    std::stringstream request;

   protected:
    httpRequest req;
    void SetUp() override {
        this->request.str ( R"(GET /path/to/resource?query=123 HTTP/1.1
Host: 123.124.123.123
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8
Accept-Language: en-US,en;q=0.5
Accept-Encoding: gzip, deflate, br
Connection: keep-alive
Cookie: session_id=123
Cookie: user_pref=dark_mode

body
of
request
more
)");
        req.parseHeader(request);
		req.parseBody(request);
    }
};

class httpBodyParseTest : public testing::Test {
   private:

   protected:
    std::stringstream request;
    httpRequest test;
    void SetUp() override {
// this->request << std::ios_base::ate << std::ios_base::in << std::ios_base::out;
        this->request.str ( R"(GET /path/to/resource?query=123 HTTP/1.1
Host: 123.124.123.123:800
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8
Accept-Language: en-US,en;q=0.5
Accept-Encoding: gzip, deflate, br
Connection: keep-alive
Cookie: session_id=123
Cookie: user_pref=dark_mode
)");
request.seekp(0, std::ios::end);
        // req.parse(request);
		// req.addToBody(request);
    }
};
