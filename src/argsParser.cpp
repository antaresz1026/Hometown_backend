/**
 * @file argsParser.cpp
 * @author antaresz (antaresz1026@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2024-10-24
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "argsParser.hpp"
#include <map>
#include <string>
#include <sstream>

std::map<std::string, std::string> argsParser::parseQuery(const std::string& query) {
    std::map<std::string, std::string> result;
    std::istringstream queryStream(query);
    std::string pair;
    
    while (std::getline(queryStream, pair, '&')) {
        size_t pos = pair.find('=');
        if (pos != std::string::npos) {
            std::string key = pair.substr(0, pos);
            std::string value = pair.substr(pos + 1);
            result[key] = value;
        }
    }
    return result;
}
