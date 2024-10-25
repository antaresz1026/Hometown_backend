/**
 * @file log.cpp
 * @author antaresz (antaresz1026@gmail.com)
 * @brief log的初始化
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
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include "logger.hpp"

bool logger::initialized = false;

void logger::log(const std::string& level, const std::string& message) {
    auto log = [](const std::string& lvl, const std::string& msg) {
        if (lvl == "info") {
            BOOST_LOG_TRIVIAL(info) << msg;
        } else if (lvl == "warning") {
            BOOST_LOG_TRIVIAL(warning) << msg;
        } else if (lvl == "error") {
            BOOST_LOG_TRIVIAL(error) << msg;
        } else if (lvl == "debug") {
            BOOST_LOG_TRIVIAL(debug) << msg;
        } else {
            BOOST_LOG_TRIVIAL(error) << "Unknown log level " << msg;
        }
    };
    log(level, message);
}

void logger::init() {
    if (!initialized) {
        // ---- 控制台输出 ----
        auto console_sink = boost::log::add_console_log(
            std::cout,
            boost::log::keywords::format = "[%TimeStamp%][%Severity%]: %Message%"
        );

        console_sink->set_filter(
            boost::log::trivial::severity >= boost::log::trivial::debug
        );

        // 设置文件输出，确保每次运行创建新的日志文件
        auto file_sink = boost::log::add_file_log(
            boost::log::keywords::file_name = "../logs/log_%Y-%m-%d_%N.log",
            boost::log::keywords::rotation_size = 10 * 1024 * 1024, // 10MB的文件轮转
            boost::log::keywords::time_based_rotation = boost::log::sinks::file::rotation_at_time_point(0, 0, 0), // 每天轮转
            boost::log::keywords::format = "[%TimeStamp%][%Severity%]: %Message%\n",
            boost::log::keywords::auto_flush = true
        );

        file_sink->set_filter(
            boost::log::trivial::severity >= boost::log::trivial::info
        );
        // 添加常用属性，如时间戳
        boost::log::add_common_attributes();
        // 标记为已初始化
        initialized = true;
    }
}