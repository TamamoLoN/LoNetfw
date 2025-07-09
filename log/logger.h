/**
 * 日志器
 */
#pragma once
#include "log/logappender.h"
#include "log/logevent.h"
#include "log/loglevel.h"
#include "util/util.h"
#include <iostream>
#include <memory>
#include <vector>

namespace lon
{
namespace log
{
class Logger
{
  public:
    using Ptr = std::shared_ptr<Logger>;
    explicit Logger(const std::string &name = "root");
    virtual ~Logger() = default;
    void log(Loglevel::Level level, LogEvent::Ptr event);
    void debug(LogEvent::Ptr event);
    void info(LogEvent::Ptr event);
    void warn(LogEvent::Ptr event);
    void error(LogEvent::Ptr event);
    void fatal(LogEvent::Ptr event);

    void addAppender(LogAppender::Ptr appender);
    void delAppender(LogAppender::Ptr appender);
    void setLevel(Loglevel::Level level);
    void setLevel(const std::string &level);
    std::string getLevel() const;
    void getLevel(Loglevel::Level &level);

    void test();

  private:
    std::string m_name;                        //日志名称
    Loglevel::Level m_level;                   //日志等级
    std::vector<LogAppender::Ptr> m_appenders; //日志输出目的地向量
};
} // namespace log
} // namespace lon