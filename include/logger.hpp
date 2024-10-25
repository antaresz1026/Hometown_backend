/**
 * @file log.hpp
 * @author antaresz (antaresz1026@gmail.com)
 * @brief log类定义
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
#ifndef _LOG_HPP
#define _LOG_HPP
#include <boost/log/trivial.hpp>

/**
 * @brief 对Boost.log的封装
 * 
 */
class logger {
public:
    // 单例模式（可选）
    static logger& getInstance() {
        static logger instance;
        return instance;
    }

    // 封装各种级别的日志输出
    void log(const std::string& level, const std::string& message);
        

private:
    logger() {
        // 初始化日志系统：控制台和文件
        init();
    }
    static void init();
    static bool initialized;
};


#endif