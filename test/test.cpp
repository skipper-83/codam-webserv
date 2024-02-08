#include <gtest/gtest.h>

#include <sstream>

#include "http_request.hpp"
#include "http_response.hpp"
#include "test_httpReq.hpp"
#include "test_config.hpp"
#include "config.hpp"
#include "path.hpp"

MainConfig mainConfig;

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
    EXPECT_NO_THROW(request.parse(req, 80));
}

// TEST(http_request_str_constructor, basic) {
//     std::string req(R"(GET /path/to/resource?query=123 HTTP/1.0
// Host: 123.124.123.123
// )");
//     httpRequest request;
//     EXPECT_NO_THROW(httpRequest test(req));
// }

TEST(http_request_fs, wrong_host) {
    std::stringstream req(R"(GET /path/to/resource?query=123 HTTP/1.0
Host: 123.124.12#3.123

)");
    EXPECT_THROW(httpRequest test(req), httpRequest::httpRequestException);
}

// TEST(http_request_str_constructor, wrong_host) {
// 	testing::internal::CaptureStderr();
//     std::string req = R"(GET /path/to/resource?query=123 HTTP/1.0
// Host: 123.124.12#3.123

// )";
// 	httpRequest test(req);
// 	std::string output = testing::internal::GetCapturedStderr();
// 	std::cerr << "output: " << output;
// 	EXPECT_TRUE(output.find("HTTP error 400: Bad Request") != std::string::npos);
// }

TEST(http_request_fs, req) {
    std::stringstream req(R"(GET /path/to/resource?query=123 HTTP/1.1
Host: 123.124.123.123
)");
    EXPECT_NO_THROW(httpRequest test(req));
}

TEST(http_request_fs, wrong_start_line1) {
    std::stringstream req(R"(GT /path/to/resource?query=123 HTTP/1.0
Host: 123.124.123.123
)");
    EXPECT_THROW(httpRequest test(req), httpRequest::httpRequestException);
}

TEST(http_request_fs, wrong_start_line2) {
    std::stringstream req(R"(GET /path/t o/resource?query=123 HTTP/1.0
Host: 123.124.123.123
)");
    EXPECT_THROW(httpRequest test(req), httpRequest::httpRequestException);
}

TEST(http_request_fs, wrong_start_line3) {
    std::stringstream req(R"(GET /path/to/resource?query=123 HTT/0.9
Host: 123.124.123.123
)");
    EXPECT_THROW(httpRequest test(req), httpRequest::httpRequestException);
}

TEST(http_request_str, wrong_start_line3) {
    std::string req = R"(GET /path/to/resource?query=123 HTT/0.9
Host: 123.124.123.123

)";
    httpRequest request;
	// testing::internal::CaptureStderr();
	EXPECT_THROW(request.parse(req, 80), httpRequest::httpRequestException);
	// std::string out = testing::internal::GetCapturedStderr();
	// EXPECT_TRUE (out.find("HTTP error 400: Bad Request") != std::string::npos);
    // EXPECT_THROW(request.parse(req), std::invalid_argument);
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
}

TEST_F(httpBodyParseTest, with_content_length)
{
	
	request << "Content-Length: 9\n\n0123456789";
	test.parseHeader(request);
	test.parseBody(request);
	EXPECT_TRUE(test.bodyComplete());
	EXPECT_EQ(test.getBodyLength(), 9);
	EXPECT_EQ(test.getBody(), "012345678");
}

TEST_F(httpBodyParseTest, empty_body)
{	
	test.parseHeader(request);
	test.parseBody(request);
	EXPECT_TRUE(test.headerComplete());
	EXPECT_EQ(test.getBodyLength(), 0);
}

TEST_F(httpBodyParseTest, with_content_length_two_chunks)
{
	
	request << "Content-Length: 9\n\n012345";
	test.parseHeader(request);
	EXPECT_TRUE(test.headerComplete());
	test.parseBody(request);
	EXPECT_FALSE(test.bodyComplete());
	EXPECT_EQ(test.getBodyLength(), 6);
	EXPECT_EQ(test.getBody(), "012345");
	request << "6789";
	test.parseBody(request);
	EXPECT_TRUE(test.bodyComplete());
	EXPECT_EQ(test.getBodyLength(), 9);
	EXPECT_EQ(test.getBody(), "012345678");

}

TEST_F(httpBodyParseTest, double_newline)
{
	
	request << "\n012345\n\n6789";
	test.parseHeader(request);
	EXPECT_TRUE(test.headerComplete());
	test.parseBody(request);
	EXPECT_TRUE(test.bodyComplete());
	EXPECT_EQ(test.getBodyLength(), 7);
	EXPECT_EQ(test.getBody(), "012345\n");
	request << "6789";
	test.parseBody(request);
	EXPECT_EQ(test.getBodyLength(), 7);
	EXPECT_EQ(test.getBody(), "012345\n");
	EXPECT_TRUE(test.bodyComplete());

}


TEST_F(httpBodyParseTest, double_newline_in_second_chunk)
{
	
	request << "\n0123456789";
	test.parseHeader(request);
	EXPECT_TRUE(test.headerComplete());
	test.parseBody(request);
	EXPECT_EQ(test.getBodyLength(), 10);
	EXPECT_EQ(test.getBody(), "0123456789");
	EXPECT_FALSE(test.bodyComplete());
	request << "joe\n\nnee";
	test.parseBody(request);
	EXPECT_EQ(test.getBodyLength(), 14);
	EXPECT_EQ(test.getBody(), "0123456789joe\n");
	EXPECT_TRUE(test.bodyComplete());

}

TEST_F(httpBodyParseTest, double_newline_is_second_chunk)
{
	
	request << "\n0123456789";
	test.parseHeader(request);
	EXPECT_TRUE(test.headerComplete());
	test.parseBody(request);
	EXPECT_EQ(test.getBodyLength(), 10);
	EXPECT_EQ(test.getBody(), "0123456789");
	EXPECT_FALSE(test.bodyComplete());
	request << "\n\nnee";
	test.parseBody(request);
	EXPECT_EQ(test.getBodyLength(), 11);
	EXPECT_EQ(test.getBody(), "0123456789\n");
	EXPECT_TRUE(test.bodyComplete());

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
	EXPECT_TRUE(test.headerComplete());
	test.parseBody(request);
	EXPECT_EQ(test.getBody(), "01234012345");
	EXPECT_EQ(test.getBodyLength(), 11);
	EXPECT_TRUE(test.bodyComplete());
}

TEST_F(httpBodyParseTest, chunked_in_three_parts)
{
	request << R"(Transfer-Encoding: chunked

5
01234)";
	test.parseHeader(request);
	EXPECT_TRUE(test.headerComplete());
	test.parseBody(request);
	EXPECT_EQ(test.getBody(), "01234");
	EXPECT_EQ(test.getBodyLength(), 5);
	request << R"(6
012345)";
	test.parseBody(request);
	EXPECT_EQ(test.getBody(), "01234012345");
	EXPECT_EQ(test.getBodyLength(), 11);
	EXPECT_EQ(test.bodyComplete(), false);
	// EXPECT_TRUE(test.bodyComplete());
	request << R"(0

and some)";
	test.parseBody(request);
	EXPECT_EQ(test.getBody(), "01234012345");
	EXPECT_EQ(test.getBodyLength(), 11);
	EXPECT_EQ(test.bodyComplete(), true);
}

TEST_F(configTest, config_happy)
{
	config_input >> config;
	EXPECT_FALSE(config.getServerFromPort(8080) == nullptr);
	EXPECT_EQ(config.getServerFromPort(1111), nullptr);
	EXPECT_EQ(config.getServerFromPortAndName(8080, "iets.localhost")->rank, 2);
	EXPECT_EQ(config.getServerFromPortAndName(8080, "blabla"), nullptr);
	EXPECT_EQ(config.getServer(8080, "blabla")->rank, 0);
}

TEST_F(configTestStub, test_defaults)
{
	const ServerConfig* server;
	AllowedMethods default_methods;
	
	config_input >> config;
	EXPECT_FALSE(config.getServerFromPort(80) == nullptr);
	server = config.getServerFromPort(80);
	EXPECT_EQ(server->clientMaxBodySize.value, DEFAULT_CLIENT_BODY_SIZE);
	EXPECT_EQ(default_methods.methods, server->allowed.methods);
	EXPECT_EQ(server->clientMaxBodySize.value, 1000000);
}

TEST_F(configTestStub, test_bracket_error)
{
	config_input << "server {";
	EXPECT_THROW(config_input >> config, std::invalid_argument);
}

TEST_F(configTestStub, test_client_max_size_error)
{
	config_input << "client_max_body_size: 10c;";
	EXPECT_THROW(config_input >> config, std::invalid_argument);
}

TEST_F(configTestStub, test_client_max_size_terminator)
{
	config_input << "client_max_body_size: 10b";
	EXPECT_THROW(config_input >> config, std::invalid_argument);
}

TEST_F(configTestStub, test_port_wrong_value)
{
	config_input << R"(server{
		listen: 70000;
})";
	EXPECT_THROW(config_input >> config, std::invalid_argument);
}

TEST_F(configTestStub, test_port_value_with_letters)
{
	config_input << R"(server{
		listen: 700aaa;
})";
	EXPECT_THROW(config_input >> config, std::invalid_argument);
}

TEST_F(configTestStub, test_error_with_no_number)
{
	config_input << R"(server{
		listen: 700;
		error_page: 404.html;
})";
	EXPECT_THROW(config_input >> config, std::invalid_argument);
}

TEST_F(configTestStub, test_duplicate_listening_port)
{
	config_input << R"(server{
		listen: 700;
		listen: 700;
})";
	EXPECT_THROW(config_input >> config, std::invalid_argument);
}
TEST_F(configWithRequest, happy_path)
{
	request_input << "Host: 123.123.123.123\n\n123456789";
	
	config_input >> config;
	request.parseHeader(request_input);
	EXPECT_TRUE(request.headerComplete());
	request.setServer(config, 8080);
	request.parseBody(request_input);
	EXPECT_EQ(request.getBody(), "123456789");
}

TEST_F(configWithRequest, request_larger_than_client_max_body_size)
{
	request_input << "Host: myname:8080\n\n123456789";
	
	config_input >> config;
	request.parseHeader(request_input);
	EXPECT_TRUE(request.headerComplete());
	request.setServer(config, 8080);
	EXPECT_THROW(request.parseBody(request_input), httpRequest::httpRequestException);
}

// TEST_F(configWithRequest, request_with_wrong_port)
// {
// 	request_input << "Host: 123.123.123.123:8081\n\n123456789";
	
// 	config_input >> config;
// 	request.parseHeader(request_input);
// 	EXPECT_TRUE(request.headerComplete());
// 	std::cerr << request.getHeaderListAsString();
// 	EXPECT_THROW(request.setServer(config, 8080), httpRequest::httpRequestException);
// }

TEST_F(configTest, ports_getter)
{
	std::stringstream ports_list;

	config_input >> config;
	for(auto it: config.getPorts())
		ports_list << it;
	EXPECT_EQ(ports_list.str(), "8080808180880");
}

// TEST(parse, basic)
// {
// 	std::string input = R"(GET )";

// 	httpRequest req;

// 	req.parse(input, 80);
// 	input.append(R"(/path/to/resource?query=123 HTTP/1.0
// Host: 123.124.123.123
// Content-Length: 9

// 123)");
// 	req.parse(input, 80);
// 	EXPECT_TRUE(req.headerComplete());
// 	EXPECT_FALSE(req.bodyComplete());
// 	EXPECT_EQ(input, "");

// 	input.append("456789abcd");
// 	req.parse(input, 80);
// 	EXPECT_TRUE(req.bodyComplete());
// 	EXPECT_EQ(input, "abcd");
// }
TEST(response, basic)
{
	httpResponse resp;
	std::string exp_eq = R"(HTTP/1.1 200 OK
Content-Length: 4
Content-Type: text/html; charset=UTF-8
Date: )" + WebServUtil::timeStamp() + "\n" + 
R"(Server: Jelle en Albert's webserv

test)";

	resp.setCode(200);
	resp.setBody("test");
	// std::cerr
	std::cerr << "Response: \n" <<  resp.getRequestAsString() << "\n";
	EXPECT_EQ(resp.getRequestAsString(), exp_eq);
}

TEST_F(configWithRequest, getErrorPage)
{
	request_input << "Host: myname:8080\n\n123456789";
	config_input >> config;

	request.parseHeader(request_input);
	request.setServer(config, 8080);
	EXPECT_EQ(request.getErrorPage(504), "/50xxxx.html");

}

TEST(path, first)
{
	checkFile("../../test/test_dir/");
}
