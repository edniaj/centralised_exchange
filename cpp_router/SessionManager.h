// SessionManager.h
#pragma once

#include <unordered_map>
#include <string>
#include <chrono>
#include "DatabaseManager.h"
#include "SocketManager.h"

class SessionManager {
private:
    struct Session {
        std::string username;
        std::string senderCompId;
        std::chrono::system_clock::time_point lastActivity;
        bool isAuthenticated;
    };

    DatabaseManager& dbManager;
    std::unordered_map<int, Session> sessions;  // socketFd -> Session
    const int SESSION_TIMEOUT;

public:
    // Constructor
    SessionManager(DatabaseManager& db);
    
    // Core functionality
    bool authenticateUser(int socketFd, const std::string& username, const std::string& password);
    bool isSessionValid(int socketFd);
    void removeSession(int socketFd);
};