#include "log.hpp"
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
/**
 * @brief 
 * 
 */
void init_log() {
    boost::log::core::get()->set_filter(
        boost::log::trivial::severity >= boost::log::trivial::info
    );
    // 将日志输出到文件，并设置日志格式
    boost::log::add_file_log(
        boost::log::keywords::file_name = "../logs/app_%N.log",  // 日志文件名
        boost::log::keywords::rotation_size = 10 * 1024 * 1024,  // 日志文件大小限制
        boost::log::keywords::time_based_rotation = boost::log::sinks::file::rotation_at_time_point(0, 0, 0),  // 每天新建日志文件
        boost::log::keywords::format = "[%TimeStamp%] [%Severity%]: %Message%"  // 日志输出格式
    );

    // 同时在控制台输出日志
    boost::log::add_console_log(
        std::cout,
        boost::log::keywords::format = "[%TimeStamp%] [%Severity%]: %Message%"
    );

    // 添加常见属性：时间戳、日志级别等
    boost::log::add_common_attributes();
}