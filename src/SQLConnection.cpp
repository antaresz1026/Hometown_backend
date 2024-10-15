/**
 * @file SQLConnection.cpp
 * @author antaresz (antaresz1026@gmail.com)
 * @brief SQLConnection类实现
 * @version 1.0
 * @date 2024-10-11
 * 
 * @copyright Copyright (c) 2024 antaresz
 * 
 * @par 修改日志:
 * <table>
 * <tr><th>Date       <th>Version <th>Author <th>Description
 * <tr><td>2024-10-11 <td>1.0     <td>antaresz    <td>desc
 * </table>
 */
#include "SQLConnection.hpp"

/**
 * @brief Construct a new SQLConnection::SQLConnection object
 * 
 * @param host 
 * @param user 
 * @param password 
 * @param database 
 */
SQLConnection::SQLConnection(const std::string& host, const std::string& user, const std::string& password, const std::string& database)
    : _host(host), _user(user), _password(password), _database(database) {
    _driver = sql::mysql::get_mysql_driver_instance();
    createConnection();
}
/**
 * @brief Destroy the SQLConnection::SQLConnection object
 * 
 */
SQLConnection::~SQLConnection() {
    while (!_connections.empty()) {
        auto conn = _connections.front();
        _connections.pop();  // 释放所有连接
    }
}
/**
 * @brief 对信号量上锁，空闲时从连接池中取出一个连接
 * 
 * @return std::shared_ptr<sql::Connection> 
 */
std::shared_ptr<sql::Connection> SQLConnection::getConnection() {
    std::unique_lock<std::mutex> lock(_mtx);
    while (_connections.empty()) {
        _cv.wait(lock);  // 等待直到有空闲连接
    }
    auto conn = _connections.front();
    _connections.pop();
    return conn;
}
/// @brief 释放连接，将其放回连接池

/// @param conn 
void SQLConnection::releaseConnection(std::shared_ptr<sql::Connection> conn) {
    std::unique_lock<std::mutex> lock(_mtx);
    _connections.push(conn);  // 将连接放回连接池
    _cv.notify_one();
}
/**
 * @brief 连接池创建
 * 
 */

void SQLConnection::createConnection() {
    for (int i = 0; i < 10; ++i) {  // 创建10个初始连接
        try {
            std::shared_ptr<sql::Connection> conn(_driver->connect(_host, _user, _password));
            conn->setSchema(_database);
            _connections.push(conn);  // 将新连接放入连接池中
        } catch (sql::SQLException& e) {
            std::cerr << "Could not connect to database. Error: " << e.what() << std::endl;
        }
    }
}
