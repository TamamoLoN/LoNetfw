#pragma once
#include "log/loglevel.h"
#include <iostream>
#include <memory>
#include <sstream>
#include <stdarg.h>
namespace lon
{
namespace log
{
/**
 * @brief 构造函数
 * @param[in] level 日志级别
 * @param[in] filename 文件名
 * @param[in] line 文件行号
 * @param[in] elapse 程序启动依赖的耗时(毫秒)
 * @param[in] thread_id 线程id
 * @param[in] fiber_id 协程id
 * @param[in] time 日志事件(秒)
 * @param[in] thread_name 线程名称
 */
class LogEvent
{
  public:
    using Ptr = std::shared_ptr<LogEvent>;
    LogEvent(LogLevel::Level level, const std::string &filename, uint32_t line, uint32_t elapse,
             uint32_t thread_id, uint32_t fiber_id, uint64_t time, std::string thread_name);
    ~LogEvent() = default;

    LogLevel::Level getLevel() const;
    std::string getFile() const;
    uint32_t getLine() const;
    uint32_t getElapse() const;
    uint32_t getThreadId() const;
    uint32_t getFiberId() const;
    uint64_t getTime() const;
    std::string getMessage() const;
    std::stringstream &getMessageStream();
    std::string getThreadName() const;

    void setMessageStream(const char *fmt, ...);
    void setMessageStream(const char *fmt, va_list al);

  private:
    // Logger::Ptr m_logger; //重新设计架构解耦
    LogLevel::Level m_level;
    std::string m_file;
    uint32_t m_line;
    uint32_t m_elapse;
    uint32_t m_thread_id;
    uint32_t m_fiber_id;
    uint64_t m_time;
    std::stringstream m_ss;
    std::string m_thread_name;
};
} // namespace log
} // namespace lon