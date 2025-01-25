#include "HttpServer.h"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

class http_session : public std::enable_shared_from_this<http_session> {
public:
    using request_handler = std::function<void(http::request<http::string_body>&,
                                             http::response<http::string_body>&)>;

    http_session(tcp::socket socket, request_handler handler)
        : socket_(std::move(socket))
        , handler_(std::move(handler))
    {
    }

    void run() {
        do_read();
    }

private:
    void do_read() {
        req_ = {};

        http::async_read(socket_, buffer_, req_,
            [self = shared_from_this()](beast::error_code ec, std::size_t) {
                if (!ec) {
                    self->create_response();
                }
            });
    }

    void create_response() {
        res_ = {};
        handler_(req_, res_);
        do_write();
    }

    void do_write() {
        http::async_write(socket_, res_,
            [self = shared_from_this()](beast::error_code ec, std::size_t) {
                self->socket_.shutdown(tcp::socket::shutdown_send, ec);
            });
    }

    tcp::socket socket_;
    beast::flat_buffer buffer_;
    http::request<http::string_body> req_;
    http::response<http::string_body> res_;
    request_handler handler_;
};

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

std::string HttpServer::generate_session_id() {
    static boost::uuids::random_generator gen;
    return boost::uuids::to_string(gen());
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
            std::string ext = path.substr(path.find_last_of(".") + 1);
            if (ext == "html") res.set(http::field::content_type, "text/html");
            else if (ext == "css") res.set(http::field::content_type, "text/css");
            else if (ext == "js") res.set(http::field::content_type, "text/javascript");
            else if (ext == "png") res.set(http::field::content_type, "image/png");
            else if (ext == "jpg" || ext == "jpeg") res.set(http::field::content_type, "image/jpeg");
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
    
    // Set response size
    res.content_length(res.body().size());
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