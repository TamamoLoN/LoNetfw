#pragma once
// #include "log/logger.h"
#include "log/logformatter.h"
#include "log/loglevel.h"
#include <iostream>
#include <memory>
#include <sstream>
namespace lon
{
namespace log
{
/**
 * @brief 构造函数
 * @param[in] logger 日志器
 * @param[in] level 日志级别
 * @param[in] file 文件名
 * @param[in] line 文件行号
 * @param[in] elapse 程序启动依赖的耗时(毫秒)
 * @param[in] thread_id 线程id
 * @param[in] fiber_id 协程id
 * @param[in] time 日志事件(秒)
 * @param[in] thread_name 线程名称
 */
class LogEvent
{
    friend class LogFormatter;

  public:
    using Ptr = std::shared_ptr<LogEvent>;
    LogEvent();
    ~LogEvent();

  private:
    // Logger::Ptr m_logger;
    Loglevel::Level m_level;
    std::string m_file;
    uint32_t m_line;
    uint32_t m_elapse;
    uint32_t m_thread_id;
    uint32_t m_fiber_id;
    time_t m_time;
    std::stringstream m_ss;
    std::string m_thread_name;
};
} // namespace log
} // namespace lon