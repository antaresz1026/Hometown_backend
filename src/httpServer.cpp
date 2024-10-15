/**
 * @file httpServer.cpp
 * @author antaresz (antaresz1026@gmail.com)
 * @brief httpServer实现
 * @version 1.0
 * @date 2024-10-13
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <boost/log/trivial.hpp>
#include <boost/bind/bind.hpp>
#include "httpServer.hpp"
httpServer::httpServer(const std::string& address, int port)
    : _io_service(), _acceptor(_io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(address), port)) {}

void httpServer::start() {
    try {
        BOOST_LOG_TRIVIAL(info) << "Accepting connections on port " << _acceptor.local_endpoint().port();
        accept();
        _io_service.run();
    } catch (const std::exception& e) {
        BOOST_LOG_TRIVIAL(error) << e.what();
    }
}

void httpServer::setRoute(const std::string& path, std::function<void(const std::string&, std::string&)> handler) {
    _routes[path] = handler;
}

void httpServer::accept() {
    // 创建一个新的 socket 来处理客户端连接
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(_io_service);
    
    // 开始异步接受连接
    _acceptor.async_accept(*socket, [this, socket](boost::system::error_code ec) {
        if (!ec) {
            // 连接成功，处理请求
            BOOST_LOG_TRIVIAL(info) << "Accepted a new connection.";
            handleRequest(socket);
        } else {
            BOOST_LOG_TRIVIAL(error) << "Error accepting connection: " << ec.message();
        }
        
        // 继续监听下一个连接
        accept();
    });
}

void httpServer::handleRequest(std::shared_ptr<boost::asio::ip::tcp::socket> socket) {
    auto buffer = std::make_shared<boost::asio::streambuf>();
    boost::asio::async_read_until(*socket, *buffer, "\r\n\r\n", 
        [this, socket, buffer](boost::system::error_code ec, std::size_t bytes_transferred) {
            if (!ec) {
                std::istream request_stream(buffer.get());
                std::string request_line;
                std::getline(request_stream, request_line);

                std::istringstream request_line_stream(request_line);
                std::string method, path, http_version;
                request_line_stream >> method >> path >> http_version;

                BOOST_LOG_TRIVIAL(info) << "Request: " << method << " " << path;

                std::string response;
                if (_routes.find(path) != _routes.end()) {
                    _routes[path](request_line, response);
                } else {
                    response = "HTTP/1.1 404 Not Found\r\n\r\n";
                }

                boost::asio::async_write(*socket, boost::asio::buffer(response),
                    [socket](boost::system::error_code ec, std::size_t /*length*/) {
                        if (!ec) {
                            BOOST_LOG_TRIVIAL(info) << "Response sent.";
                        }
                        socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
                        socket->close();
                    });
            } else {
                BOOST_LOG_TRIVIAL(error) << "Error reading request: " << ec.message();
            }
        });
}
