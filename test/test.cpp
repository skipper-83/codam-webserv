#include <gtest/gtest.h>

#include <sstream>

#include "hello-world.hpp"
#include "http_request.hpp"
#include "test_httpReq.hpp"
#include "config.hpp"

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
    EXPECT_EQ(req.getBody(), "body\nof\nrequest\nmore\n");
}

TEST_F(httpRequestTest, get_header) {
    EXPECT_EQ(req.getHeader("Accept-Language"), "en-US,en;q=0.5");
}

TEST_F(httpRequestTest, get_headerlist) {
    EXPECT_EQ(req.getHeaderList("Cookie").at(0), "session_id=123");
    EXPECT_EQ(req.getHeaderList("Cookie").at(1), "user_pref=dark_mode");
    // std::cout "BOOH"\n
}

TEST_F(httpBodyParseTest, with_content_length)
{
	
	request << "Content-Length: 9\n\n0123456789";
	test.parseHeader(request);
	test.addToBody(request);
	EXPECT_EQ(test.getBodyLength(), 9);
	EXPECT_EQ(test.getBody(), "012345678");
}

TEST_F(httpBodyParseTest, with_content_length_two_chunks)
{
	
	request << "Content-Length: 9\n\n012345";
	test.parseHeader(request);
	test.addToBody(request);
	EXPECT_EQ(test.getBodyLength(), 6);
	EXPECT_EQ(test.getBody(), "012345");
	request << "6789";
	test.addToBody(request);
	EXPECT_EQ(test.getBodyLength(), 9);
	EXPECT_EQ(test.getBody(), "012345678");

}

TEST_F(httpBodyParseTest, double_newline)
{
	
	request << "\n012345\n\n6789";
	std::cerr << request.str();
	test.parseHeader(request);
	test.addToBody(request);
	EXPECT_EQ(test.getBodyLength(), 7);
	EXPECT_EQ(test.getBody(), "012345\n");
	request << "6789";
	test.addToBody(request);
	EXPECT_EQ(test.getBodyLength(), 7);
	EXPECT_EQ(test.getBody(), "012345\n");

}


TEST_F(httpBodyParseTest, double_newline_in_second_chunk)
{
	
	request << "\n0123456789";
	std::cerr << request.str();
	test.parseHeader(request);
	test.addToBody(request);
	EXPECT_EQ(test.getBodyLength(), 10);
	EXPECT_EQ(test.getBody(), "0123456789");
	request << "joe\n\nnee";
	test.addToBody(request);
	EXPECT_EQ(test.getBodyLength(), 14);
	EXPECT_EQ(test.getBody(), "0123456789joe\n");

}

TEST_F(httpBodyParseTest, double_newline_is_second_chunk)
{
	
	request << "\n0123456789";
	std::cerr << request.str();
	test.parseHeader(request);
	test.addToBody(request);
	EXPECT_EQ(test.getBodyLength(), 10);
	EXPECT_EQ(test.getBody(), "0123456789");
	request << "\n\nnee";
	test.addToBody(request);
	// EXPECT_EQ(test.getBodyLength(), 11);
	// EXPECT_EQ(test.getBody(), "0123456789\n");

}

TEST_F(httpBodyParseTest, chunked_happy)
{
	request << R"(Transfer-Encoding: chunked

5
01234
6
012345
0

GET etc)";
	test.parseHeader(request);
	test.addToBody(request);
	EXPECT_EQ(test.getBody(), "01234012345");
	EXPECT_EQ(test.getBodyLength(), 11);
}

TEST_F(httpBodyParseTest, chunked_in_three_parts)
{
	request << R"(Transfer-Encoding: chunked

5
01234)";
	test.parseHeader(request);
	test.addToBody(request);
	EXPECT_EQ(test.getBody(), "01234");
	EXPECT_EQ(test.getBodyLength(), 5);
	request << R"(6
012345)";
	test.addToBody(request);
	EXPECT_EQ(test.getBody(), "01234012345");
	EXPECT_EQ(test.getBodyLength(), 11);
	EXPECT_EQ(test.bodyComplete(), false);
	request << R"(0

and some)";
	test.addToBody(request);
	EXPECT_EQ(test.getBody(), "01234012345");
	EXPECT_EQ(test.getBodyLength(), 11);
	EXPECT_EQ(test.bodyComplete(), true);
	std::cerr << "HOST: " << test.getHeader("Host") << "\n";
}

TEST(config, first_test)
{
	// std::cout << argv[1];
	std::fstream file;
	MainConfig config;

	// if (!file)
		file.open("../../test/test.conf");
	// if (file){
	file >> config;
	EXPECT_FALSE(config.getServerFromPort(8080) == nullptr);
	EXPECT_EQ(config.getServerFromPort(1111), nullptr);
	EXPECT_EQ(config.getServerFromPortAndName(8080, "iets.localhost")->rank, 2);
	EXPECT_EQ(config.getServerFromPortAndName(8080, "blabla"), nullptr);
	EXPECT_EQ(config.getServer(8080, "blabla")->rank, 0);
	// }
	// std::cerr << 
}

