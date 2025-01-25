#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <string>
#include <filesystem>

class HttpServer {
public:
    explicit HttpServer(int port);
    void start();

private:
    void do_accept();
    void handle_request(boost::beast::http::request<boost::beast::http::string_body>& req,
                       boost::beast::http::response<boost::beast::http::string_body>& res);
    std::string load_static_file(const std::string& path);
    std::string generate_session_id();
    
    int port;
    boost::asio::io_context ioc;
    boost::asio::ip::tcp::acceptor acceptor;
    std::filesystem::path static_root;  // Path to static files
};

#endif 