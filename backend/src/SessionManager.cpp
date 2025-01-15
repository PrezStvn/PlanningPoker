// src/SessionManager.cpp
#include "SessionManager.h"
#include <iostream>
#include <sstream>
#include <functional>

SessionManager::SessionManager() {
    // Initialize the command map with function pointers
    commandHandlers = {
        {"vote", [this](const std::string& userId, const std::string& content) {
            handleVote(userId, content);
        }},
        {"post", [this](const std::string& userId, const std::string& content) {
            handlePost(userId, content);
        }},
        {"setname", [this](const std::string& userId, const std::string& newName) {
            handleSetName(userId, newName);
        }}
        // Add more commands here as needed
    };
}

void SessionManager::handleVote(const std::string& userId, const std::string& voteValue) {
    currentVotes[userId] = voteValue;
    std::cout << "Vote received from " << userId << ": " << voteValue << std::endl;
}

void SessionManager::handlePost(const std::string& userId, const std::string& message) {
    std::string formattedMessage = userId + ": " + message;
    messageHistory.push_back(formattedMessage);
    std::cout << "Post received: " << formattedMessage << std::endl;
}

void SessionManager::handleSetName(const std::string& userId, const std::string& newName) {
    userNames[userId] = newName;
    std::cout << "User " << userId << " changed name to: " << newName << std::endl;
}

void SessionManager::handleMessage(const std::string& message) {
    std::istringstream iss(message);
    std::string command;
    
    // Get the command (everything before the ':')
    if (std::getline(iss, command, ':')) {
        std::string content;
        std::getline(iss, content);  // Get the rest of the message
        
        // Remove leading whitespace from content
        content.erase(0, content.find_first_not_of(" \t\n\r\f\v"));
        
        // Assuming userId is passed as part of the content for now
        std::string userId = "temp-user-id";  // This should come from connection info
        
        // Look up the command in our map and execute if found
        auto handlerIt = commandHandlers.find(command);
        if (handlerIt != commandHandlers.end()) {
            handlerIt->second(userId, content);
        } else {
            std::cout << "Unknown command: " << command << std::endl;
        }
    }
}
