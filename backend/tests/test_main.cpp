// tests/test_main.cpp
#include <iostream>
#include "WebSocketServer.h"
#include "SessionManager.h"

int main() {
    std::cout << "Running tests..." << std::endl;

    // Example test
    WebSocketServer server(8080);
    std::cout << "Server initialized on port " << 8080 << std::endl;

    return 0;
}
