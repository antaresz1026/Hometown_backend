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
#include "userHandler.hpp"

userHandler::userHandler(SQLConnection& connectionPool) : _connection_pool(connectionPool) {}

// 使用 Crypto++ 生成 SHA256 加密密码，并将其与盐一起存储
std::string userHandler::encryptPassword(const std::string& password) {
    using namespace CryptoPP;

    // 生成一个随机的盐值
    std::string salt = generateSalt();

    // 使用 HMAC-SHA256 进行加密
    std::string key = salt;  // 盐值作为密钥
    std::string digest;
    HMAC<SHA256> hmac((byte*)key.data(), key.size());

    // 使用 Crypto++ 计算 HMAC-SHA256 的值
    StringSource(password, true,
                 new HashFilter(hmac,
                                new HexEncoder(new StringSink(digest))));

    // 返回 盐值:加密后的密码 的格式，方便存储和验证
    return salt + ":" + digest;
}

// 验证密码是否匹配
bool userHandler::verifyPassword(const std::string& password, const std::string& encrypted) {
    using namespace CryptoPP;

    // 提取盐值和存储的加密密码
    size_t pos = encrypted.find(':');
    if (pos == std::string::npos) {
        return false;  // 格式不正确
    }

    std::string salt = encrypted.substr(0, pos);
    std::string storedPassword = encrypted.substr(pos + 1);

    // 使用盐值加密输入的密码
    std::string key = salt;
    std::string digest;
    HMAC<SHA256> hmac((byte*)key.data(), key.size());

    // 计算 HMAC-SHA256 的值
    StringSource(password, true,
                 new HashFilter(hmac,
                                new HexEncoder(new StringSink(digest))));

    // 比较加密后的密码和存储的密码是否一致
    return digest == storedPassword;
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
bool userHandler::registerUser(const std::string& username, const std::string& password) {
    auto conn = _connection_pool.getConnection();
    std::string encryptedPassword = encryptPassword(password);

    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement("INSERT INTO users (username, password) VALUES (?, ?)"));
        pstmt->setString(1, username);
        pstmt->setString(2, encryptedPassword);
        pstmt->executeUpdate();
        _connection_pool.releaseConnection(conn);
        return true;
    } catch (sql::SQLException& e) {
        std::cerr << "Registration failed. Error: " << e.what() << std::endl;
        _connection_pool.releaseConnection(conn);
        return false;
    }
}

// 用户登录
bool userHandler::loginUser(const std::string& username, const std::string& password) {
    auto conn = _connection_pool.getConnection();
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement("SELECT password FROM users WHERE username = ?"));
        pstmt->setString(1, username);
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        if (res->next()) {  // 查询结果有匹配的用户名
            std::string storedPassword = res->getString("password");
            _connection_pool.releaseConnection(conn);
            return verifyPassword(password, storedPassword);
        }

        _connection_pool.releaseConnection(conn);
    } catch (sql::SQLException& e) {
        std::cerr << "Login failed. Error: " << e.what() << std::endl;
        _connection_pool.releaseConnection(conn);
    }
    return false;
}