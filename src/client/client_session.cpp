#include "client.hpp"
#include "logging.hpp"

static CPPLog::Instance sessionLogI = logOut.instance(CPPLog::Level::DEBUG, "WebServSession");

void Client::_handleSession() {
    sessionLogI << "Trying to find session" << CPPLog::end;
    sessionLogI << "Cookie Header: " << _request.getHeader("Cookie") << CPPLog::end;
    if (_session == nullptr) {
        sessionLogI << "No session yet" << CPPLog::end;
        if (!_request.getCookie(SESSION_COOKIE_NAME).empty()) {
            try {
                _session = _sessionList.getSession(_request.getCookie(SESSION_COOKIE_NAME));
                sessionLogI << "Session found in list: " << _session->getSessionId() << CPPLog::end;
            } catch (const std::exception& e) {
                sessionLogI << "Session not found in list: " << e.what() << CPPLog::end;
                _session = _sessionList.createSession();
                sessionLogI << "Session added: " << CPPLog::end;
                sessionLogI << _session->getSessionId() << CPPLog::end;
                _session->setSessionIdToResponse(_response);
                sessionLogI << "Session added: " << _session->getSessionId() << CPPLog::end;
            }
        } else {
            sessionLogI << "No session cookie" << CPPLog::end;
            _session = _sessionList.createSession();
            _session->setSessionIdToResponse(_response);
            sessionLogI << "Session added: " << _session->getSessionId() << CPPLog::end;
        }
        changeState(ClientState::READ_BODY);
    } else {
        sessionLogI << "Session already exists" << CPPLog::end;
    }
    _session->addPathToTrail(_request.getAdress());  // add path to session trail
    _request.setSession(true);                       // set session flag to true to avoid duplication on longer requests
}
