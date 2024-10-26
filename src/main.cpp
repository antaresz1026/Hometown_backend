/**
 * @file main.cpp
 * @author antaresz (antaresz1026@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2024-10-24
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include <nlohmann/json.hpp>
#include "httpsServer.hpp"
#include "userHandler.hpp"
#include "SQLConnection.hpp"
#include "logger.hpp"

int main() {
    SQLConnection sql_connection("localhost", "antaresz", "antaresz.cc", "hometown");
    httpsServer server;
    userHandler user_handler(sql_connection);

    server.setRoute("/register", [&user_handler](const std::string& request, std::string& response) {
        // 从 request 提取用户名和密码，调用 userHandler.registerUser 处理注册逻辑。
        try {
            auto json_body = nlohmann::json::parse(request);
            std::string username = json_body["username"];
            std::string password = json_body["password"];

            if(user_handler.registerUser(username, password)) {
                response = "HTTP/1.1 200 OK\r\n\r\n";
                response += "Welcome, " + username + "!";
            } else {
                response = "HTTP/1.1 200 OK\r\n\r\n";
                response += "Sorry, something wrong happend when registing.";
            }
        } catch (const nlohmann::json::exception& e) {
            // 构造错误响应
            response = "HTTP/1.1 400 Bad Request\r\n\r\nInvalid JSON: ";
            response += e.what();
        }
    });

    server.setRoute("/login", [&user_handler](const std::string& request, std::string& response) {
        try {
            auto json_body = nlohmann::json::parse(request);
            std::string username = json_body["username"];
            std::string password = json_body["password"];

            if(user_handler.loginUser(username, password)) {
                response = "HTTP/1.1 200 OK\r\n\r\nLogin successful";
            } else {
                response = "HTTP/1.1 401 Unauthorized\r\n\r\nLogin failed";
            }
        } catch (const nlohmann::json::exception& e) {
            // 构造错误响应
            response = "HTTP/1.1 400 Bad Request\r\n\r\nInvalid JSON: ";
            response += e.what();
        }
    });
    server.start();
    return 0;
}