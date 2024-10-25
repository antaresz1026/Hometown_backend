/**
 * @file argsParser.hpp
 * @author antaresz (antaresz1026@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2024-10-24
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef _ARGSPARSER_HPP
#define _ARGSPARSER_HPP

#include <map>
#include <string>

class argsParser {
public:
    std::map<std::string, std::string> parseQuery(const std::string& query);
};

#endif