#include <gtest/gtest.h>

#include <sstream>

#include "hello-world.hpp"
#include "http_request.hpp"
#include "test_httpReq.hpp"

TEST(hello_world, basic) {
    EXPECT_EQ(hello_world(), "Hello, World!");
}

TEST(http_request_fs, basic) {
    std::stringstream req(R"(GET /path/to/resource?query=123 HTTP/1.0
Host: 123.124.123.123
)");
    ;
    EXPECT_NO_THROW(httpRequest test(req));
}

TEST(http_request_str, basic) {
    std::string req(R"(GET /path/to/resource?query=123 HTTP/1.0
Host: 123.124.123.123
)");
    httpRequest request;
    EXPECT_NO_THROW(request.parse(req));
}

TEST(http_request_str_constructor, basic) {
    std::string req(R"(GET /path/to/resource?query=123 HTTP/1.0
Host: 123.124.123.123
)");
    httpRequest request;
    EXPECT_NO_THROW(httpRequest test(req));
}

TEST(http_request_fs, wrong_host) {
    std::stringstream req(R"(GET /path/to/resource?query=123 HTTP/1.0
	Host: 123.124.12#3.123
	)");
    EXPECT_THROW(httpRequest test(req), std::invalid_argument);
}

TEST(http_request_str_constructor, wrong_host) {
    std::string req = R"(GET /path/to/resource?query=123 HTTP/1.0
	Host: 123.124.12#3.123
	)";
    EXPECT_THROW(httpRequest test(req), std::invalid_argument);
}

TEST(http_request_fs, req) {
    std::stringstream req(R"(GET /path/to/resource?query=123 HTTP/1.1
	Host: 123.124.123.123
	)");
    EXPECT_THROW(httpRequest test(req), std::invalid_argument);
}

TEST(http_request_fs, wrong_start_line1) {
    std::stringstream req(R"(GT /path/to/resource?query=123 HTTP/1.0
Host: 123.124.123.123
)");
    EXPECT_THROW(httpRequest test(req), std::invalid_argument);
}

TEST(http_request_fs, wrong_start_line2) {
    std::stringstream req(R"(GET /path/t o/resource?query=123 HTTP/1.0
Host: 123.124.123.123
)");
    EXPECT_THROW(httpRequest test(req), std::invalid_argument);
}

TEST(http_request_fs, wrong_start_line3) {
    std::stringstream req(R"(GET /path/to/resource?query=123 HTT/0.9
Host: 123.124.123.123
)");
    EXPECT_THROW(httpRequest test(req), std::invalid_argument);
}

TEST(http_request_str, wrong_start_line3) {
    std::string req = R"(GET /path/to/resource?query=123 HTT/0.9
Host: 123.124.123.123
)";
    httpRequest request;
    EXPECT_THROW(request.parse(req), std::invalid_argument);
}

TEST_F(httpRequestTest, get_adress) {
    EXPECT_EQ(req.getAdress(), "/path/to/resource?query=123");
}

TEST_F(httpRequestTest, get_request_type) {
    EXPECT_EQ(req.getRequestType(), "GET");
}

TEST_F(httpRequestTest, get_protocol) {
    EXPECT_EQ(req.getProtocol(), "HTTP/1.1");
}

TEST_F(httpRequestTest, get_body) {
    EXPECT_EQ(req.getBody(), "body\nof\nrequest\n\nmore\n");
}

TEST_F(httpRequestTest, get_header) {
    EXPECT_EQ(req.getHeader("Accept-Language"), "en-US,en;q=0.5");
}

TEST(http_request_fs, add_to_body){
//    req.addToBody(this->request);
    httpRequest request;

    std::stringstream piet(R"(GET /path/to/resource?query=123 HTTP/1.1
Host: 123.124.123.123
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8
Accept-Language: en-US,en;q=0.5
Accept-Encoding: gzip, deflate, br
Connection: keep-alive
Cookie: session_id=123
Cookie: user_pref=dark_mode
Content-Length: 27

body
of
request
more
)", std::ios_base::ate | std::ios_base::in | std::ios_base::out);
    // request.parse(req);
    // std::cerr << "PIET1\n" << piet.str();
    request.parse(piet);
    //  std::cerr << "PIET2\n" << piet.str();
    request.addToBody(piet);
    piet << "MOOOORE";
    request.addToBody(piet);
    // std::cerr << request;
    // std::cerr << "\n\n\n" << req << "\n\n\nyoyoyo\n\n";
}


TEST_F(httpRequestTest, get_headerlist) {
    EXPECT_EQ(req.getHeaderList("Cookie").at(0), "session_id=123");
    EXPECT_EQ(req.getHeaderList("Cookie").at(1), "user_pref=dark_mode");
    // std::cout "BOOH"\n
}