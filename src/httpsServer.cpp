/**
 * @file httpServer.cpp
 * @author antaresz (antaresz1026@gmail.com)
 * @brief httpServer类实现
 * @version 1.0
 * @date 2024-10-15
 * 
 * @copyright Copyright (c) 2024 antaresz
 * 
 * @par 修改日志:
 * <table>
 * <tr><th>Date       <th>Version <th>Author <th>Description
 * <tr><td>2024-10-15 <td>1.0     <td>antaresz    <td>desc
 * </table>
 */
#include <boost/log/trivial.hpp>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <nlohmann/json.hpp> 
#include <iostream>
#include "httpsServer.hpp"
#include "logger.hpp"

/// @brief httpServer运行在localhost的23030端口上

httpsServer::httpsServer()
    : _io_service(), _acceptor(_io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string("0.0.0.0"), PORT)), _ssl_context(boost::asio::ssl::context::tlsv12_server), 
    _cert_path("/etc/letsencrypt/live/antaresz.cc/fullchain.pem"), _key_path("/etc/letsencrypt/live/antaresz.cc/privkey.pem") {
    _ssl_context.use_certificate_chain_file(_cert_path);
    _ssl_context.use_private_key_file(_key_path, boost::asio::ssl::context::pem);

    logger::getInstance().log("info", "HTTPS Server initialized.");
}
/**
 * @brief 规定启动逻辑，先accept然后io_service.run
 * 
 */
void httpsServer::start() {
    boost::asio::signal_set signals(_io_service, SIGINT, SIGTERM);
    signals.async_wait([this](boost::system::error_code /*ec*/, int /*signo*/) {
        _io_service.stop();
        logger::getInstance().log("info", "Server stopped.");
    });

    accept();
    _io_service.run();
}
/**
 * @brief 设置_routes表的(path,handler)对
 * 
 * @param path 
 * @param handler 
 */
void httpsServer::setRoute(const std::string& path, std::function<void(const std::string&, std::string&)> handler) {
    _routes[path] = handler;
    logger::getInstance().log("debug", "Route set for: " + path);
}

std::string httpsServer::simulateRequest(const std::string& method, const std::string& path, const std::string& body) {
    std::string response;

    // 模拟路由查找和请求处理
    if (_routes.find(path) != _routes.end()) {
        _routes[path](body, response);
    } else {
        response = "HTTP/1.1 404 Not Found\r\n\r\n";
    }

    return response;
}

/**
 * @brief accept逻辑，先创建一个socket，异步接受连接
 * 
 */
void httpsServer::accept() {
    // 创建一个新的 TCP socket
    auto tcp_socket = std::make_shared<boost::asio::ip::tcp::socket>(_io_service);

    // 开始异步接受连接
    _acceptor.async_accept(*tcp_socket,
        [this, tcp_socket](boost::system::error_code ec) {
            if (!ec) {
                // 将 TCP socket 封装到 SSL stream 中
                auto ssl_socket = std::make_shared<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>(std::move(*tcp_socket), _ssl_context);

                // 开始 SSL 握手
                ssl_socket->async_handshake(boost::asio::ssl::stream_base::server,
                    [this, ssl_socket](const boost::system::error_code& ec) {
                        if (!ec) {
                            logger::getInstance().log("info", "Accepted a new connection.") ;
                            handleRequest(ssl_socket);
                        } else {
                            std::string msg = "Handshake failed: " + ec.message();

                            logger::getInstance().log("error", msg) ;
                        }
                    });
            } else {
                std::string msg = "Handshake failed: " + ec.message();

                logger::getInstance().log("error", msg) ;
            }

            // 准备接受下一个连接
            accept();
        });
}
/**
 * @brief 处理请求逻辑
 * 
 * @param socket 
 */
void httpsServer::handleRequest(std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket) {
    auto buffer = std::make_shared<boost::asio::streambuf>();

    // 第一步：读取请求头
    boost::asio::async_read_until(*socket, *buffer, "\r\n\r\n",
        [this, socket, buffer](boost::system::error_code ec, std::size_t bytes_transferred) {
            if (!ec) {
                std::istream request_stream(buffer.get());
                std::string request_line;
                std::getline(request_stream, request_line);

                std::istringstream request_line_stream(request_line);
                std::string method, path, http_version;
                request_line_stream >> method >> path >> http_version;

                // 查找 Content-Length
                std::string header;
                std::size_t content_length = 0;
                while (std::getline(request_stream, header) && header != "\r") {
                    if (header.find("Content-Length:") != std::string::npos) {
                        content_length = std::stoul(header.substr(16));
                    }
                }

                logger::getInstance().log("debug", "Request: " + method + " " + path + 
                                          ", Content-Length: " + std::to_string(content_length));

                if (content_length > 0) {
                    // 第二步：如果有请求体，则读取剩余部分
                    auto body_buffer = std::make_shared<std::vector<char>>(content_length);
                    boost::asio::async_read(*socket, boost::asio::buffer(*body_buffer, content_length), boost::asio::transfer_exactly(content_length), 
                        [this, socket, body_buffer, path](boost::system::error_code ec, std::size_t length) {
                            std::string response;
                            std::cout << "1 \n";

                            if (!ec && length == body_buffer->size()) {
                                std::cout << "2 \n";
                                std::string body(body_buffer->data(), length);
                                logger::getInstance().log("debug", "Request body: " + body);

                                if (_routes.find(path) != _routes.end()) {
                                    _routes[path](body, response);
                                } else {
                                    response = "HTTP/1.1 404 Not Found\r\n\r\n";
                                }

                                // 发送响应并关闭连接
                                boost::asio::async_write(*socket, boost::asio::buffer(response),
                                    [socket](boost::system::error_code ec, std::size_t /*length*/) {
                                        std::cout << "3 \n";
                                        socket->async_shutdown([socket](boost::system::error_code ec) {
                                            if (!ec) {
                                                socket->lowest_layer().close();
                                            } else {
                                                logger::getInstance().log("error", "Error shutting down SSL: " + ec.message());
                                            }
                                        });
                                    });
                            } else {
                                response = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
                                logger::getInstance().log("error", "Error reading body: " + ec.message());
                            }
                        });
                } else {
                    // 如果没有请求体，直接返回响应
                    std::string response = "HTTP/1.1 200 OK\r\n\r\n";
                    boost::asio::async_write(*socket, boost::asio::buffer(response),
                        [socket](boost::system::error_code ec, std::size_t /*length*/) {
                            socket->async_shutdown([socket](boost::system::error_code ec) {
                                if (!ec) {
                                    socket->lowest_layer().close();
                                } else {
                                    logger::getInstance().log("error", 
                                        "Error shutting down SSL: " + ec.message());
                                }
                            });
                        });
                }
            } else {
                logger::getInstance().log("error", "Error reading request: " + ec.message());
            }
        });
}
