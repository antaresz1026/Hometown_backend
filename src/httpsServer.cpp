/**
 * @file httpServer.cpp
 * @author antaresz (antaresz1026@gmail.com)
 * @brief httpServer类实现
 * @version 1.1
 * @date 2024-10-15
 * 
 * @copyright Copyright (c) 2024 antaresz
 * 
 * @par 修改日志:
 * <table>
 * <tr><th>Date       <th>Version <th>Author <th>Description
 * <tr><td>2024-10-15 <td>1.0     <td>antaresz    <td>desc
 * <tr><td>2024-10-26 <td>1.1     <td>antaresz    <td>fk缓冲区，fk all
 * </table>
 */
#include <boost/log/trivial.hpp>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <nlohmann/json.hpp> 
#include <assert.h>
#include <iostream>
#include "httpsServer.hpp"
#include "logger.hpp"

/**
 * @brief httpServer运行在localhost的23030端口上
 */
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
 * @brief 请求头处理
 * 
 * @param socket 
 */
void httpsServer::handleRequest(std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket) {
    auto buffer = std::make_shared<boost::asio::streambuf>();

    // 异步读取请求头，直到找到 "\r\n\r\n"
    boost::asio::async_read_until(*socket, *buffer, "\r\n\r\n",
        [this, socket, buffer](boost::system::error_code ec, std::size_t bytes_transferred) {
            if (!ec) {
                std::string msg_display(boost::asio::buffers_begin(buffer->data()), boost::asio::buffers_end(buffer->data()));
                logger::getInstance().log("debug", "Raw request: " + msg_display);
                // 将请求头数据转换为字符串
                std::string raw_data(boost::asio::buffers_begin(buffer->data()), boost::asio::buffers_begin(buffer->data()) + bytes_transferred);

                // 消费已读取的数据
                buffer->consume(bytes_transferred);

                // 解析请求头
                std::istringstream request_stream(raw_data);
                std::string method, path, http_version;
                request_stream >> method >> path >> http_version;


                // 查找 Content-Length
                std::string header;
                std::size_t content_length = 0;
                std::getline(request_stream, header);
                assert(header != "\r");
                while (std::getline(request_stream, header) && header != "\r") {
                    std::string header_lower = header;
                    std::transform(header_lower.begin(), header_lower.end(), header_lower.begin(), ::tolower);
                    std::cout << "Each header: " << header;

                    if (header_lower.find("content-length:") != std::string::npos) {
                        try {
                            content_length = std::stoul(header.substr(16));
                            logger::getInstance().log("debug", "Parsed Content-Length: " + std::to_string(content_length));         //log不支持流操作符，因此需要将content_length转换为字符串
                        } catch (const std::exception& e) {
                            logger::getInstance().log("error", "Invalid Content-Length: " + std::string(e.what()));
                            sendResponse(socket, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n");
                            return;
                        }
                    }
                }
                std::cout << "wtf!!!!!!!!!!!!!!!!!!" << buffer->size() << " " << content_length << "\n";
                bool received_body = buffer->size() - content_length >= 0 ? true : false;                                           //>0代表收到了下一个包的部分，=0代表刚好收到

                if (received_body) {
                    // 读取请求体数据
                    std::string body(
                        boost::asio::buffers_begin(buffer->data()),
                        boost::asio::buffers_begin(buffer->data()) + buffer->size());
                    logger::getInstance().log("debug", "RequestBody: " + body);
                    processRequest(socket, path, body);
                } else {
                    // 读取剩余的请求体
                    readBody(socket, buffer, content_length - buffer->size(), path);
                }
            } else {
                logger::getInstance().log("error", "Error reading headers: " + ec.message());
                sendResponse(socket, "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n");
            }
        });
}

/**
 * @brief 读取请求体，当请求头和请求体分开发送时才会触发
 * 
 * @param socket 
 * @param buffer 
 * @param bytes_to_read 剩余要读取的字节数
 * @param path 
 */
void httpsServer::readBody(std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket, std::shared_ptr<boost::asio::streambuf> buffer, std::size_t bytes_to_read, const std::string& path) {
    logger::getInstance().log("debug", "Reading body, bytes to read: " + std::to_string(bytes_to_read));

    // 异步读取剩余请求体
    boost::asio::async_read(*socket, buffer->prepare(bytes_to_read), boost::asio::transfer_exactly(bytes_to_read),
        [this, socket, buffer, path](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                buffer->commit(length);  // 提交读取的数据

                // 从缓冲区提取完整的请求体
                std::istream body_stream(buffer.get());
                std::string body((std::istreambuf_iterator<char>(body_stream)), std::istreambuf_iterator<char>());

                logger::getInstance().log("debug", "Received body (from async_read): " + body);

                // 处理请求并发送响应
                processRequest(socket, path, body);
            } else {
                logger::getInstance().log("error", "Error reading body: " + ec.message());
                sendResponse(socket, "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n");
            }
        });
}

/**
 * @brief 路由匹配
 * 
 * @param socket 
 * @param path 
 * @param body 
 */
void httpsServer::processRequest(std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket, const std::string& path, const std::string& body) {
    std::string response;

    if (_routes.find(path) != _routes.end()) {
        _routes[path](body, response);
    } else {
        response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
    }
    //这里要注意如果路由对应的处理函数没有设置response的情况。
    sendResponse(socket, response);
}

/**
 * @brief 发送请求
 * 
 * @param socket 
 * @param response 
 */
void httpsServer::sendResponse(std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket, const std::string& response) {
    logger::getInstance().log("debug", "Sending response: " + response);

    boost::asio::async_write(*socket, boost::asio::buffer(response),
        [socket](boost::system::error_code ec, std::size_t /*length*/) {
            if (!ec) {
                logger::getInstance().log("info", "Response sent successfully."); 
                socket->async_shutdown([socket](boost::system::error_code ec) {
                    if (!ec) {
                        socket->lowest_layer().close();
                    } else {
                        logger::getInstance().log("error", "Error shutting down SSL: " + ec.message());
                    }
                });
            } else {
                logger::getInstance().log("error", "Error sending response: " + ec.message());
            }
        });
}