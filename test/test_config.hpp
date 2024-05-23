#include <gtest/gtest.h>
#include "httpMessage/http_request.hpp"
#include "config.hpp"

#include <iostream>
#include <sstream>

class configTest : public testing::Test {
   private:

   protected:
    std::stringstream config_input;
	MainConfig config;
    void SetUp() override {
        this->config_input.str ( R"(#error_log  logs/error.log; #error_log  logs/error.log  notice;
#Specifies the file where server logs. 

# events {
#     worker_connections  1024;
#     # worker_processes and worker_connections allows you to calculate maxclients value: 
#     # max_clients = worker_processes * worker_connections
# }

# http {
#     include       mime.types;
# client_max_body_size 1g ;
# If serving locally stored static files, sendfile is essential to speed up the server,
# But if using as reverse proxy one can deactivate it

# keepalive_timeout  65;
autoindex on;
client_max_body_size 22;
allowed_methods GET POST;
server {
		listen 8080;
		listen 8081;
		listen 808;
		location / {
						root   /usr/share/nginx/html ;
						index  index3.html index2.htm ;
		}
}
server {
# listen       8080;

server_name  localhost *.localhost;
autoindex off;
allowed_methods OPTIONS;
location / {
	root   /usr/share/nginx/html;
	index  index.html index.htm;
}

error_page  404              /404.html;

error_page   500 502 503 504  /50xxxx.html;
location  /50x.html {
	root   /usr/share/nginx/html;
}
	}
server {
		listen 8080;
		server_name iets.localhost;
		location / {
						root   /usr/share/nginx/html;
						index  index2.html;
		}
}

	server {
		listen 8080;
		server_name *.j-projects.nl;
		location / {
						root   /usr/share/nginx/html;
						index  jelle.html;
		}
		autoindex on;
	client_max_body_size 0.91g;
	cgi .php .phtml /usr/bin/php-cgi;
	cgi .py /usr/bin/python3;
}


# another virtual host using mix of IP-, name-, and port-based configuration
#
#server {
#    listen       8000;
#    listen       somename:8080;
#    server_name  somename  alias  another.alias;

#    location / {
#        root   html;
#        index  index.html index.htm;
#    }
#}


# HTTPS server
#
#server {
#    listen       443 ssl;
#    server_name  localhost;

#    ssl_certificate      cert.pem;
#    ssl_certificate_key  cert.key;

#    ssl_session_cache    shared:SSL:1m;
#    ssl_session_timeout  5m;

#    ssl_ciphers  HIGH:!aNULL:!MD5;
#    ssl_prefer_server_ciphers  on;

#    location / {
#        root   html;
#        index  index.html index.htm;
#    }
#}
#ditniet

# }

)");
    }
};

class configTestStub : public testing::Test {
   private:

   protected:
    std::stringstream config_input;
	MainConfig config;
    void SetUp() override {
        this->config_input.str ( R"(#
server {
		location / {
						root   /usr/share/nginx/html ;
						index  index3.html index2.htm ;
		}
	}
)");
    }
};

class configWithRequest : public testing::Test {
   private:

   protected:
    std::stringstream config_input;
	std::stringstream request_input;
	MainConfig config;
    httpRequest request;
    void SetUp() override {
        this->config_input.str ( R"(#
server {
				listen 8080;
location / {
				root   /usr/share/nginx/html ;
				index  index3.html index2.htm ;
			}
}
server {
				listen 8080;
				server_name myname;
	location / {
				root   /usr/share/nginx/html ;
				index  index3.html index2.htm ;
				}
				error_page   500 502 503 504  /50xxxx.html;
				error_page  404              /404.html;
				client_max_body_size 5;
	}
)");
 this->request_input.str ( R"(GET /path/to/resource?query=123 HTTP/1.1
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8
Accept-Language: en-US,en;q=0.5
Accept-Encoding: gzip, deflate, br
Connection: keep-alive
Cookie: session_id=123
)");
request_input.seekp(0, std::ios::end);
    }
	
};