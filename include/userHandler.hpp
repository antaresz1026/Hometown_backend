/**
 * @file userHandler.hpp
 * @author antaresz (antaresz1026@gmail.com)
 * @brief 注册/登录api
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
#ifndef _USERHANDLER_HPP
#define _USERHANDLER_HPP

#include "SQLConnection.hpp"
#include <string>

class userHandler {
public:
    userHandler(SQLConnection& connectionPool);
    bool registerUser(const std::string& username, const std::string& password);
    bool loginUser(const std::string& username, const std::string& password);

private:
    SQLConnection& _connection_pool;
    std::string encryptPassword(const std::string& password);
    bool verifyPassword(const std::string& password, const std::string& encrypted);
    
    std::string generateSalt();  // 生成盐值
    const size_t SALT_LENGTH = 16;  // 盐的长度
};

#endif // USERHANDLER_HPP
