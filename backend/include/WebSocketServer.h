// include/WebSocketServer.h
#ifndef WEBSOCKETSERVER_H
#define WEBSOCKETSERVER_H

#include <functional>
#include <string>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <nlohmann/json.hpp>
#include <map>
#include <mutex>
#include <memory>
#include <chrono>
#include <vector>

class WebSocketServer {
public:
    explicit WebSocketServer(int port);
    
    // Start the server with message handler callback
    void start(const std::function<void(const std::string&)>& onMessage);
    
    // Send message to specific connection
    void send(const std::string& conn_id, const std::string& message);
    
    // Broadcast message to all connections except sender
    void broadcast(const std::string& sender_id, const std::string& message);
    
    // Get all active connection IDs
    std::vector<std::string> getActiveConnections() const;

private:
    void do_accept();
    void on_accept(boost::beast::error_code ec, boost::asio::ip::tcp::socket socket);
    void do_session(boost::beast::websocket::stream<boost::asio::ip::tcp::socket> ws);
    std::string generate_uuid();

    int port;
    boost::asio::io_context ioc;
    boost::asio::ip::tcp::acceptor acceptor;
    std::function<void(const std::string&)> messageHandler;
    
    // Connection tracking
    struct Connection {
        std::shared_ptr<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>> ws;
        std::string userId;  // Can be set later for user identification
        std::chrono::steady_clock::time_point lastActivity;
    };
    
    std::map<std::string, Connection> connections;
    mutable std::mutex connections_mutex;
};

#endif // WEBSOCKETSERVER_H
