#include <gtest/gtest.h>

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
Content-Length: 12


body
of
request

more
)");
        req.parse(request);
    }
    // void TearDown() override { delete this.req; }
};

// class httpStreamTest : public testing::Test {
//    private:
//     std::stringstream request;

//    protected:
//     httpRequest req;
//     void SetUp() override {
//         this->request.str ( R"(GET /path/to/resource?query=123 HTTP/1.1
// Host: 123.124.123.123
// User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.36
// Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8
// Accept-Language: en-US,en;q=0.5
// Accept-Encoding: gzip, deflate, br
// Connection: keep-alive
// Cookie: session_id=123
// Cookie: user_pref=dark_mode
// Content-Length: 12


// body
// of
// request

// more
// )");
//         req.parse(request);
//         std::cerr << "done\n";
//         request << "even more";
//         std::cerr << request;
//         // req.addToBody(request);
//     }
//     // void TearDown() override { delete this.req; }
// };