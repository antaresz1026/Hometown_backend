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
#include "argsParser.hpp"

int main() {
    SQLConnection sql_connection("localhost", "antaresz", "antaresz.cc", "hometown");
    httpsServer server;
    userHandler user_handler(sql_connection);
    argsParser parser;

    server.setRoute("/register", [&user_handler, &parser](const std::string& request, std::string& response) {
        // 从 request 提取用户名和密码，调用 userHandler.registerUser 处理注册逻辑。
        try {
            // 使用 nlohmann/json 解析 body 数据
            auto json_body = nlohmann::json::parse(request);
            std::string username = json_body["username"];
            std::string password = json_body["password"];

            std::cout << username << " " << password;
            // 构造成功响应
            response = "HTTP/1.1 200 OK\r\n\r\n";
            response += "Welcome, " + username + "!";
        } catch (const nlohmann::json::exception& e) {
            // 构造错误响应
            response = "HTTP/1.1 400 Bad Request\r\n\r\nInvalid JSON: ";
            response += e.what();
        }
    });

    server.setRoute("/login", [&user_handler, &parser](const std::string& request, std::string& response) {
        // 从 request 提取用户名和密码，调用 userHandler.loginUser 处理登录逻辑。
        std::string query = request.substr(request.find('?') + 1);
        auto params = parser.parseQuery(query);

        if (params.count("username") && params.count("password")) {
            bool success = user_handler.loginUser(params["username"], params["password"]);
            response = success ? "HTTP/1.1 200 OK\r\n\r\nLogin successful"
                            : "HTTP/1.1 401 Unauthorized\r\n\r\nLogin failed";
        } else {
            response = "HTTP/1.1 400 Bad Request\r\n\r\nMissing username or password";
        }
    });
    server.start();
    return 0;
}