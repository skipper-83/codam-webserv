#pragma once

#include <string>
#include <map>
#include <chrono>
#include "http_response.hpp"


class WebServSession {

	friend class WebServSessionList;

	public:
		WebServSession();
		~WebServSession();
		WebServSession(const WebServSession &rhs);
		WebServSession &operator=(const WebServSession &rhs);

		std::string getSessionId() const;
		void setSessionIdToResponse(httpResponse &response);
		// void setSessionDataToResponse(httpResponse &response);
		void updateSessionTimeout();
		void addPathToTrail(std::string path);

	private:
		std::string generateSessionId();
		std::string _sessionId;
		std::string _pathTrail = "";
		// std::map<std::string, std::string> _sessionData;
		std::chrono::time_point<std::chrono::steady_clock> _lastActivityTime = std::chrono::steady_clock::now();		
};

class WebServSessionList {
	public:
		WebServSessionList();
		~WebServSessionList();
		WebServSessionList(const WebServSessionList &rhs);
		WebServSessionList &operator=(const WebServSessionList &rhs);

		std::shared_ptr<WebServSession> getSession(std::string sessionId);
		std::shared_ptr<WebServSession> createSession();
		void removeSession(std::string sessionId);
		void removeExpiredSessions(std::chrono::time_point<std::chrono::steady_clock> now);

	private:
		std::vector<std::shared_ptr<WebServSession>> _sessions;
};

