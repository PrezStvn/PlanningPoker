// include/SessionManager.h
#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <nlohmann/json.hpp>

class SessionManager {
public:
    SessionManager();
    
    // Single parameter message handler
    void handleMessage(const std::string& jsonMessage);

private:
    // Command handlers
    void handleVote(const std::string& userId, const std::string& voteValue);
    void handlePost(const std::string& userId, const std::string& message);
    void handleSetName(const std::string& userId, const std::string& newName);

    // Storage
    std::map<std::string, std::string> currentVotes;  // userId -> voteValue
    std::map<std::string, std::string> userNames;     // userId -> displayName
    std::vector<std::string> messageHistory;
    std::map<std::string, std::string> userConnections;  // connectionId -> userId

    // Command map type: command -> handler function
    using CommandHandler = std::function<void(const std::string&, const std::string&)>;
    std::map<std::string, CommandHandler> commandHandlers;
};

#endif
       //
