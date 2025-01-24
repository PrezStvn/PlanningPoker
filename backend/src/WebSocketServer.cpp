// src/WebSocketServer.cpp
#include "WebSocketServer.h"
#include <iostream>
#include <boost/beast/websocket.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <nlohmann/json.hpp>

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

WebSocketServer::WebSocketServer(int port)
    : port(port)
    , ioc()
    , acceptor(ioc, {net::ip::make_address("0.0.0.0"), static_cast<unsigned short>(port)})
{
}

void WebSocketServer::start(const std::function<void(const std::string&)>& onMessage) {
    messageHandler = onMessage;
    
    std::cout << "WebSocket server starting on port " << port << std::endl;
    
    // Start accepting connections
    do_accept();
    
    // Run the I/O service
    ioc.run();
}

void WebSocketServer::do_accept() {
    acceptor.async_accept(
        [this](beast::error_code ec, tcp::socket socket) {
            on_accept(ec, std::move(socket));
        });
}

void WebSocketServer::on_accept(beast::error_code ec, tcp::socket socket) {
    if (ec) {
        std::cerr << "Accept error: " << ec.message() << std::endl;
    } else {
        // Create a WebSocket stream from the socket
        std::make_shared<websocket::stream<tcp::socket>>(std::move(socket));
        
        // Set up a new session
        do_session(websocket::stream<tcp::socket>(std::move(socket)));
    }
    
    // Accept the next connection
    do_accept();
}

std::string WebSocketServer::generate_uuid() {
    static boost::uuids::random_generator gen;
    return boost::uuids::to_string(gen());
}

void WebSocketServer::do_session(websocket::stream<tcp::socket> ws) {
    // Generate unique ID for this connection
    std::string conn_id = generate_uuid();
    
    // Store the connection
    {
        std::lock_guard<std::mutex> lock(connections_mutex);
        connections[conn_id] = Connection{
            std::make_shared<websocket::stream<tcp::socket>>(std::move(ws)),
            "",  // Empty userId initially
            std::chrono::steady_clock::now()
        };
    }

    std::thread([this, conn_id]() mutable {
        try {
            auto& conn = connections[conn_id];
            auto& ws = *conn.ws;
            ws.accept();
            
            // Send connection ID to client
            nlohmann::json connectMsg = {
                {"type", "connection_established"},
                {"connection_id", conn_id}
            };
            ws.write(net::buffer(connectMsg.dump()));
            
            beast::flat_buffer buffer;
            while (true) {
                ws.read(buffer);
                std::string message = beast::buffers_to_string(buffer.data());
                buffer.consume(buffer.size());
                
                // Update last activity
                conn.lastActivity = std::chrono::steady_clock::now();
                
                if (messageHandler) {
                    // Prepend connection ID to message
                    nlohmann::json msgJson = nlohmann::json::parse(message);
                    msgJson["connection_id"] = conn_id;
                    messageHandler(msgJson.dump());
                }
            }
        }
        catch (beast::system_error const& se) {
            if (se.code() != websocket::error::closed) {
                std::cerr << "WebSocket error: " << se.code().message() << std::endl;
            }
            // Remove connection on disconnect
            std::lock_guard<std::mutex> lock(connections_mutex);
            connections.erase(conn_id);
        }
    }).detach();
}

void WebSocketServer::send(const std::string& conn_id, const std::string& message) {
    std::lock_guard<std::mutex> lock(connections_mutex);
    if (auto it = connections.find(conn_id); it != connections.end()) {
        it->second.ws->write(net::buffer(message));
    }
}

void WebSocketServer::broadcast(const std::string& sender_id, const std::string& message) {
    std::lock_guard<std::mutex> lock(connections_mutex);
    for (const auto& [conn_id, conn] : connections) {
        if (conn_id != sender_id) {
            conn.ws->write(net::buffer(message));
        }
    }
}

std::vector<std::string> WebSocketServer::getActiveConnections() const {
    std::lock_guard<std::mutex> lock(connections_mutex);
    std::vector<std::string> active_connections;
    for (const auto& [conn_id, conn] : connections) {
        active_connections.push_back(conn_id);
    }
    return active_connections;
}

