/**
 * @brief 日志格式器
 * @param[in] format 格式模板
 * @details
 *  %m 消息
 *  %p 日志级别
 *  %r 累计毫秒数
 *  %c 日志名称
 *  %t 线程id
 *  %n 换行
 *  %d 时间
 *  %f 文件名
 *  %l 行号
 *  %T 制表符
 *  %F 协程id
 *  %N 线程名称
 *
 *  默认格式 "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
 */
#pragma once

#include "log/logformatitem.h"
#include <functional>
#include <map>
#include <memory>
#include <vector>

namespace lon
{
namespace log
{
class LogFormatter
{
  public:
    using Ptr = std::shared_ptr<LogFormatter>;
    explicit LogFormatter(const std::string &format = "");
    virtual ~LogFormatter() = default;
    std::string format(std::string logger_name, Loglevel::Level level, LogEvent::Ptr event);
    void setFormat(const std::string &format);
    std::string getFormat() const;

  private:
    std::vector<LogFormatItem::Ptr> m_items;
    std::string m_format;
    std::map<std::string, std::function<LogFormatItem::Ptr(const std::string &str)>> m_item_factory;
};
} // namespace log
} // namespace lon