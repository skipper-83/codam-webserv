#include "session.hpp"

#include <random>

#include "config.hpp"
#include "logging.hpp"

static CPPLog::Instance infoLog = logOut.instance(CPPLog::Level::INFO, "WebServSession");

WebServSession::WebServSession() {
    _sessionId = generateSessionId();
    infoLog << "Session created: " << _sessionId << CPPLog::end;
}

WebServSession::WebServSession(const WebServSession& rhs) {
    infoLog << "Session copied" << CPPLog::end;
    *this = rhs;
}

WebServSession& WebServSession::operator=(const WebServSession& rhs) {
    infoLog << "Session assigned" << CPPLog::end;
    if (this == &rhs)
        return *this;
    _sessionId = rhs._sessionId;
    // _sessionData = rhs._sessionData;
    _lastActivityTime = rhs._lastActivityTime;
    _pathTrail = rhs._pathTrail;
    return *this;
}

WebServSession::~WebServSession() {
    infoLog << "Session destroyed" << CPPLog::end;
}

std::string WebServSession::getSessionId() const {
    return _sessionId;
}

void WebServSession::setSessionIdToResponse(httpResponse& response) {
    response.setHeader("Set-Cookie", SESSION_COOKIE_NAME "=" + _sessionId + "; Path=/");
}

void WebServSession::updateSessionTimeout() {
    _lastActivityTime = std::chrono::steady_clock::now();
}

void WebServSession::addPathToTrail(std::string path) {
    _pathTrail += path + "\n";
    infoLog << "Path trail: " << _pathTrail << CPPLog::end;
}

std::string WebServSession::generateSessionId() {
    std::string sessionId;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 61);
    for (int i = 0; i < 32; i++) {
        int r = dis(gen);
        if (r < 10)
            sessionId.push_back('0' + r);
        else if (r < 36)
            sessionId.push_back('A' + r - 10);
        else
            sessionId.push_back('a' + r - 36);
    }
    return sessionId;
}

WebServSessionList::WebServSessionList() {
    infoLog << "Session list created" << CPPLog::end;
}

WebServSessionList::WebServSessionList(const WebServSessionList& rhs) {
    infoLog << "Session list copied" << CPPLog::end;
    *this = rhs;
}

WebServSessionList& WebServSessionList::operator=(const WebServSessionList& rhs) {
    infoLog << "Session list assigned" << CPPLog::end;
    if (this == &rhs)
        return *this;
    _sessions = rhs._sessions;
    return *this;
}

WebServSessionList::~WebServSessionList() {
    infoLog << "Session list destroyed" << CPPLog::end;
}

std::shared_ptr<WebServSession> WebServSessionList::getSession(std::string sessionId) {
    auto it = std::find_if(_sessions.begin(), _sessions.end(),
                           [sessionId](const std::shared_ptr<WebServSession> session) { return session->getSessionId() == sessionId; });
    if (it == _sessions.end())
        throw std::runtime_error("Session not found");
    return (*it);
}

std::shared_ptr<WebServSession> WebServSessionList::createSession() {
    infoLog << "Creating new session" << CPPLog::end;
    _sessions.push_back(std::make_shared<WebServSession>());
    return (_sessions.back());
}

void WebServSessionList::removeSession(std::string sessionId) {
    _sessions.erase(std::remove_if(_sessions.begin(), _sessions.end(),
                                   [sessionId](const std::shared_ptr<WebServSession> session) { return session->getSessionId() == sessionId; }),
                    _sessions.end());
}

void WebServSessionList::removeExpiredSessions(std::chrono::time_point<std::chrono::steady_clock> now) {
    _sessions.erase(std::remove_if(_sessions.begin(), _sessions.end(),
                                   [now, this](const std::shared_ptr<WebServSession> session) {
                                       bool expired = (now - session->_lastActivityTime) > std::chrono::seconds(DEFAULT_TIMEOUT_SECONDS);
                                       if (expired) {
                                           infoLog << "Session expired: " << session->getSessionId()
                                                   << " session list length before deletion: " << this->_sessions.size() << CPPLog::end;
                                           infoLog << "Path trail: " << session->_pathTrail << CPPLog::end;
                                       }
                                       return (expired);
                                   }),
                    _sessions.end());
}
