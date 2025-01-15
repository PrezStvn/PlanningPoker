// include/WebSocketServer.h
#ifndef WEBSOCKETSERVER_H
#define WEBSOCKETSERVER_H

#include <functional>
#include <string>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <nlohmann/json.hpp>

class WebSocketServer {
public:
    explicit WebSocketServer(int port);
    void start(const std::function<void(const std::string&)>& onMessage);

private:
    void do_accept();
    void on_accept(boost::beast::error_code ec, boost::asio::ip::tcp::socket socket);
    void do_session(boost::beast::websocket::stream<boost::asio::ip::tcp::socket> ws);

    int port;
    boost::asio::io_context ioc;
    boost::asio::ip::tcp::acceptor acceptor;
    std::function<void(const std::string&)> messageHandler;
};

#endif // WEBSOCKETSERVER_H
