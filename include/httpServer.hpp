/**
 * @file httpServer.hpp
 * @author antaresz (antaresz1026@gmail.com)
 * @brief httpServer类声明
 * @version 1.0
 * @date 2024-10-08
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef _HTTP_SERVER_HPP
#define _HTTP_SERVER_HPP

#include <boost/asio.hpp>
#include <string>
#include <functional>
#include <map>

#define PORT 3030
/**
 * @brief httpServer类
 * 
 */
class httpServer {
public:
    /**
     * @brief 设置httpServer的地址和端口
     * 
     * @param address 
     * @param port 
     */
    httpServer(const std::string& address, int port);
    void start();
    /**
     * @brief 设置路由
     * 
     * @param path 
     * @param handler 
     */
    void setRoute(const std::string& path, std::function<void(const std::string&, std::string&)> handler);

private:
    void accept();
    /**
     * @brief 对request的解析和response的设置
     * 
     * @param request 
     * @param response 
     */
    void handleRequest(std::shared_ptr<boost::asio::ip::tcp::socket> socket);

    boost::asio::io_service _io_service;
    boost::asio::ip::tcp::acceptor _acceptor;
    std::map<std::string, std::function<void(const std::string&, std::string&)>> _routes;
};

#endif