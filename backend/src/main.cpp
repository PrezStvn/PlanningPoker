// src/main.cpp
#include <iostream>
#include "WebSocketServer.h"
#include "HttpServer.h"
#include "SessionManager.h"
#include <thread>

int main() {
    std::cout << "Starting Planning Poker Backend..." << std::endl;

    // Create instances of the servers and session manager
    HttpServer httpServer(8081);        // HTTP for static content
    WebSocketServer wsServer(8082);     // WebSocket for real-time communication
    SessionManager sessionManager;

    // Start the WebSocket server in a separate thread
    std::thread wsThread([&wsServer, &sessionManager]() {
        wsServer.start([&sessionManager](const std::string& message) {
            sessionManager.handleMessage(message);
        });
    });

    // Start the HTTP server in the main thread
    httpServer.start();  // This will block

    wsThread.join();
    return 0;
}
