// src/WebSocketServer.cpp
#include "WebSocketServer.h"
#include <iostream>
#include <boost/beast/websocket.hpp>

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

void WebSocketServer::do_session(websocket::stream<tcp::socket> ws) {
    // Launch a new thread for each connection
    std::thread([this, ws = std::move(ws)]() mutable {
        try {
            // Accept the websocket handshake
            ws.accept();
            
            // Buffer for reading messages
            beast::flat_buffer buffer;
            
            // Keep reading messages until an error occurs
            while (true) {
                // Read a message
                ws.read(buffer);
                
                // Extract the message as a string
                std::string message = beast::buffers_to_string(buffer.data());
                buffer.consume(buffer.size());
                
                // Handle the message
                if (messageHandler) {
                    messageHandler(message);
                }
                
                // Echo the message back (you might want to modify this)
                ws.write(net::buffer(message));
            }
        }
        catch (beast::system_error const& se) {
            if (se.code() != websocket::error::closed) {
                std::cerr << "WebSocket error: " << se.code().message() << std::endl;
            }
        }
        catch (std::exception const& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }).detach();
}
