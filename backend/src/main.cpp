// src/main.cpp
#include <iostream>
#include "WebSocketServer.h"
#include "SessionManager.h"

int main() {
    std::cout << "Starting Planning Poker Backend..." << std::endl;

    // Create instances of the WebSocket server and session manager
    WebSocketServer server(8080);
    SessionManager sessionManager;

    // Start the WebSocket server and define the lambda function to handle incoming messages
    server.start([&sessionManager](const std::string& message) {
        std::cout << "Received message: " << message << std::endl;
        sessionManager.handleMessage(message);
    });

    return 0;
}
