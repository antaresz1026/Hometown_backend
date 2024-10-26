/**
 * @file httpsServer.hpp
 * @author antaresz (antaresz1026@gmail.com)
 * @brief httpsServer类定义
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
#ifndef _HTTPSSERVER_HPP
#define _HTTPSSERVER_HPP

#include <boost/asio.hpp>
#include <string>
#include <functional>
#include <map>
#include <boost/asio/ssl.hpp>

#define PORT 23030
/**
 * @brief httpsServer类
 * 
 */
class httpsServer {
public:
    /**
     * @brief httpsServer初始化
     * 
     */
    httpsServer();
    /**
     * @brief httpsServer启动
     * 
     */
    void start();
    /**
     * @brief 设置路由
     * 
     * @param path 
     * @param handler 
     */
    void setRoute(const std::string& path, std::function<void(const std::string&, std::string&)> handler);
    std::string simulateRequest(const std::string& method, const std::string& path, const std::string& body = "");
private:
    /**
     * @brief 接受socket逻辑
     * 
     */
    void accept();
    /**
     * @brief 对socket处理
     * 
     * @param socket 
     */
    void handleRequest(std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket);
    void readBody(std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket, std::shared_ptr<boost::asio::streambuf> buffer, std::size_t content_length, const std::string& path);
    void sendResponse(std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket, const std::string& response);
    void processRequest(std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket, const std::string& path, const std::string& body);
    boost::asio::io_service _io_service;                                                        //io_service
    boost::asio::ip::tcp::acceptor _acceptor;                                                   //acceptor接收器
    boost::asio::ssl::context _ssl_context;                                                     //ssl
    std::string _cert_path;                                                                     //证书目录
    std::string _key_path;                                                                      //密钥目录
    std::map<std::string, std::function<void(const std::string&, std::string&)>> _routes;       //路由
};

#endif