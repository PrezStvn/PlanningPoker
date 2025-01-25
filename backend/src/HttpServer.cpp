#include "HttpServer.h"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

HttpServer::HttpServer(int port)
    : port(port)
    , ioc()
    , acceptor(ioc, {net::ip::make_address("0.0.0.0"), static_cast<unsigned short>(port)})
    , static_root("../static")  // Path to static files directory
{
}

void HttpServer::start() {
    std::cout << "HTTP server starting on port " << port << std::endl;
    do_accept();
    ioc.run();
}

void HttpServer::do_accept() {
    acceptor.async_accept(
        [this](beast::error_code ec, tcp::socket socket) {
            if (!ec) {
                // Create the HTTP session
                std::make_shared<http_session>(std::move(socket), 
                    [this](http::request<http::string_body>& req, 
                           http::response<http::string_body>& res) {
                        handle_request(req, res);
                    })->run();
            }
            
            // Accept another connection
            do_accept();
        });
}

void HttpServer::handle_request(http::request<http::string_body>& req,
                              http::response<http::string_body>& res) {
    // Set up response defaults
    res.version(req.version());
    res.keep_alive(req.keep_alive());
    
    // Route the request
    if (req.method() == http::verb::get) {
        if (req.target() == "/") {
            // Serve landing page
            res.set(http::field::content_type, "text/html");
            res.body() = load_static_file("/index.html");
        }
        else if (req.target().starts_with("/static/")) {
            // Serve static files
            std::string path = std::string(req.target());
            res.body() = load_static_file(path);
            
            // Set content type based on file extension
            if (path.ends_with(".html")) res.set(http::field::content_type, "text/html");
            else if (path.ends_with(".css")) res.set(http::field::content_type, "text/css");
            else if (path.ends_with(".js")) res.set(http::field::content_type, "text/javascript");
            else if (path.ends_with(".png")) res.set(http::field::content_type, "image/png");
            else if (path.ends_with(".jpg")) res.set(http::field::content_type, "image/jpeg");
        }
        else if (req.target().starts_with("/session/")) {
            // Serve session page
            res.set(http::field::content_type, "text/html");
            res.body() = load_static_file("/session.html");
        }
    }
    else if (req.method() == http::verb::post) {
        if (req.target() == "/session/create") {
            // Create new session
            std::string sessionId = generate_session_id();
            
            nlohmann::json response = {
                {"sessionId", sessionId}
            };
            
            res.set(http::field::content_type, "application/json");
            res.body() = response.dump();
        }
    }
    
    // Set response size and send
    res.set(http::field::content_length, res.body().size());
    res.result(http::status::ok);
}

std::string HttpServer::load_static_file(const std::string& path) {
    std::filesystem::path filepath = static_root / path.substr(1);
    std::ifstream file(filepath);
    if (!file) {
        return "404 - File not found";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
} 