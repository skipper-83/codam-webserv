#ifndef HTTP_REQUEST_HPP
# define HTTP_REQUEST_HPP

# include <map>
# include <iostream>
# include <fstream>
# include <sstream>
# include <regex>

class httpRequest {
	private:
		void								_getHttpHeaders(std::ifstream &fs);
		void								_getHttpStartLine(std::ifstream &fs);
		void								_checkHttpHeaders(void);
		std::string							_httpRequestType;
		std::string							_httpAdress;
		std::string							_httpProtocol;
		std::map<std::string, std::string>	_httpHeaders;
		std::string							_httpRequestBody;

	public:
						httpRequest(std::ifstream &fs);
						~httpRequest();
		std::string		getAdress(void) const;
		std::string		getRequestType(void) const;
		std::string		getProtocol(void) const;
		std::string		getHeader(const std::string &key) const;
		std::string		getBody(void) const;
		void			printHeaders(std::ostream &os) const;
};

std::ostream&	operator<<(std::ostream& os, httpRequest const& t);

#endif