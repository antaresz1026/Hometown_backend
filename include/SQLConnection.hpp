/**
 * @file SQLConnection.hpp
 * @author antaresz (antaresz1026@gmail.com)
 * @brief SQLConnection类声明定义
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
#ifndef _SQLCONNECTION_HPP
#define _SQLCONNECTION_HPP

#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <string>
#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>

class SQLConnection {
public:
    SQLConnection(const std::string& host, const std::string& user, const std::string& password, const std::string& database);
    ~SQLConnection();

    std::shared_ptr<sql::Connection> getConnection();
    void releaseConnection(std::shared_ptr<sql::Connection> conn);

private:
    void createConnection();

    std::string _host;
    std::string _user;
    std::string _password;
    std::string _database;

    std::queue<std::shared_ptr<sql::Connection>> _connections;
    std::mutex _mtx;
    std::condition_variable _cv;
    sql::mysql::MySQL_Driver* _driver;
};

#endif // SQLCONNECTION_HPP
