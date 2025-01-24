// src/SessionManager.cpp
#include "SessionManager.h"
#include <iostream>
#include <sstream>
#include <functional>
#include <nlohmann/json.hpp>

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
    std::string userName = userNames.count(userId) ? userNames[userId] : userId;
    std::string formattedMessage = userName + ": " + message;
    messageHistory.push_back(formattedMessage);
    std::cout << "Post received: " << formattedMessage << std::endl;
}

void SessionManager::handleSetName(const std::string& userId, const std::string& newName) {
    userNames[userId] = newName;
    std::cout << "User " << userId << " changed name to: " << newName << std::endl;
}

void SessionManager::handleMessage(const std::string& jsonMessage) {
    try {
        // Parse the JSON message
        nlohmann::json msgJson = nlohmann::json::parse(jsonMessage);
        
        // Extract connection_id and command type
        std::string connectionId = msgJson["connection_id"];
        std::string command = msgJson["type"];
        
        // Get or create userId for this connection
        std::string userId;
        if (userConnections.count(connectionId)) {
            userId = userConnections[connectionId];
        } else {
            // If this is a new connection, create a temporary userId
            userId = "user-" + connectionId.substr(0, 8);  // Use first 8 chars of connection ID
            userConnections[connectionId] = userId;
        }
        
        // Extract content based on command type
        std::string content;
        if (msgJson.contains("content")) {
            content = msgJson["content"];
        }
        
        // Look up the command in our map and execute if found
        auto handlerIt = commandHandlers.find(command);
        if (handlerIt != commandHandlers.end()) {
            handlerIt->second(userId, content);
        } else {
            std::cout << "Unknown command: " << command << std::endl;
        }
    }
    catch (const nlohmann::json::exception& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error processing message: " << e.what() << std::endl;
    }
}
