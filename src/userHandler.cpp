/**
 * @file userHandler.cpp
 * @author antaresz (antaresz1026@gmail.com)
 * @brief 注册/登录api实现
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
#include <cppconn/prepared_statement.h>
#include <cryptopp/sha.h>
#include <cryptopp/hmac.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include <cryptopp/osrng.h>
#include <cryptopp/base64.h>
#include <sstream>
#include <iomanip>
#include "userHandler.hpp"

userHandler::userHandler(SQLConnection& connectionPool) : _connection_pool(connectionPool) {}

// 修改后的 encryptPassword，接受盐值作为参数
std::string userHandler::encryptPassword(const std::string& password, const std::string& salt) {
    using namespace CryptoPP;

    std::string key = salt;  // 使用给定的盐值作为密钥
    std::string digest;
    HMAC<SHA256> hmac((byte*)key.data(), key.size());

    // 计算 HMAC-SHA256 的值
    StringSource(password, true,
                 new HashFilter(hmac,
                                new HexEncoder(new StringSink(digest))));

    return digest;
}

// 生成一个随机盐值，用于密码加密
std::string userHandler::generateSalt() {
    using namespace CryptoPP;

    AutoSeededRandomPool rng;
    byte salt[SALT_LENGTH];
    rng.GenerateBlock(salt, sizeof(salt));

    // 将盐值转换为十六进制字符串
    std::ostringstream oss;
    for (size_t i = 0; i < SALT_LENGTH; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)salt[i];
    }
    return oss.str();
}

// 注册用户
bool userHandler::registerUser(const std::string& username, const std::string& password, const std::string& user_type, const std::string& id_type, const std::string& id_number, const std::string& phone) {
    auto conn = _connection_pool.getConnection();

    std::string salt = generateSalt();
    std::string encryptedPassword = encryptPassword(password, salt);

    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement("INSERT INTO users (username, salt, password, user_type, id_type, id_number, phone) VALUES (?, ?, ?, ?, ?, ?, ?)")
        );
        pstmt->setString(1, username);
        pstmt->setString(2, salt);
        pstmt->setString(3, encryptedPassword);
        pstmt->setString(4, user_type);
        pstmt->setString(5, id_type);
        pstmt->setString(6, id_number);
        pstmt->setString(7, phone);
        pstmt->executeUpdate();
        _connection_pool.releaseConnection(conn);
        return true;
    } catch (sql::SQLException& e) {
        std::cerr << "Registration failed. Error: " << e.what() << std::endl;
        _connection_pool.releaseConnection(conn);
        return false;
    }
}

// 登录用户
bool userHandler::loginUser(const std::string& username, const std::string& password) {
    auto conn = _connection_pool.getConnection();
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement("SELECT salt, password FROM users WHERE username = ?")
        );
        pstmt->setString(1, username);
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        if (res->next()) {  // 找到匹配的用户
            std::string salt = res->getString("salt");
            std::string storedPassword = res->getString("password");
            _connection_pool.releaseConnection(conn);

            // 使用数据库中的盐值加密输入的密码
            std::string encryptedPassword = encryptPassword(password, salt);

            // 比较加密后的密码和存储的密码是否一致
            return encryptedPassword == storedPassword;
        }

        _connection_pool.releaseConnection(conn);
    } catch (sql::SQLException& e) {
        std::cerr << "Login failed. Error: " << e.what() << std::endl;
        _connection_pool.releaseConnection(conn);
    }
    return false;
}