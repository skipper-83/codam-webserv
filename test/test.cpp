#include <gtest/gtest.h>

#include <sstream>

#include "hello-world.hpp"
#include "http_request.hpp"
#include "test_httpReq.hpp"

TEST(hello_world, basic) {
    EXPECT_EQ(hello_world(), "Hello, World!");
}

TEST(http_request, basic) {
    std::stringstream req(R"(GET /path/to/resource?query=123 HTTP/1.0
Host: 123.124.123.123
)");
    ;
    EXPECT_NO_THROW(httpRequest test(req));
}

TEST(http_request, wrong_host) {
    std::stringstream req(R"(GET /path/to/resource?query=123 HTTP/1.0
	Host: 123.124.12#3.123
	)");
    EXPECT_THROW(httpRequest test(req), std::invalid_argument);
}

TEST(http_request, req) {
    std::stringstream req(R"(GET /path/to/resource?query=123 HTTP/1.1
	Host: 123.124.123.123
	)");
    EXPECT_THROW(httpRequest test(req), std::invalid_argument);
}

TEST(http_request, wrong_start_line1) {
    std::stringstream req(R"(GT /path/to/resource?query=123 HTTP/1.0
Host: 123.124.123.123
)");
    EXPECT_THROW(httpRequest test(req), std::invalid_argument);
}

TEST(http_request, wrong_start_line2) {
    std::stringstream req(R"(GET /path/t o/resource?query=123 HTTP/1.0
Host: 123.124.123.123
)");
    EXPECT_THROW(httpRequest test(req), std::invalid_argument);
}
TEST(http_request, wrong_start_line3) {
    std::stringstream req(R"(GET /path/to/resource?query=123 HTT/0.9
Host: 123.124.123.123
)");
    EXPECT_THROW(httpRequest test(req), std::invalid_argument);
}

TEST_F(httpRequestTest, get_adress) {
    EXPECT_EQ(req->getAdress(), "/path/to/resource?query=123");
}

TEST_F(httpRequestTest, get_request_type) {
    EXPECT_EQ(req->getRequestType(), "GET");
}

TEST_F(httpRequestTest, get_protocol) {
    EXPECT_EQ(req->getProtocol(), "HTTP/1.1");
}

TEST_F(httpRequestTest, get_body) {
    EXPECT_EQ(req->getBody(), "body\nof\nrequest\n\n");
}

TEST_F(httpRequestTest, get_header) {
    EXPECT_EQ(req->getHeader("Accept-Language"), "en-US,en;q=0.5");
}

TEST_F(httpRequestTest, get_headerlist) {
    EXPECT_EQ(req->getHeaderList("Cookie").at(0), "session_id=123");
    EXPECT_EQ(req->getHeaderList("Cookie").at(1), "user_pref=dark_mode");
}