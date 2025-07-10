/**
 * 日志器
 */
#pragma once
#include "log/logappender.h"
#include "log/logevent.h"
#include "log/loglevel.h"
#include "log/logmacro.h"
#include "util/singleton.h"
#include "util/util.h"
#include <iostream>
#include <memory>
#include <vector>

namespace lon
{
namespace log
{
class Logger : public std::enable_shared_from_this<Logger>
{
  public:
    using Ptr = std::shared_ptr<Logger>;
    explicit Logger(const std::string &name = "root",
                    Loglevel::Level level   = Loglevel::Level::DEBUG);
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
    Loglevel::Level getLevel() const;
    void getLevel(std::string &level);
    std::string getName() const;

    void test();

  private:
    std::string m_name;                        //日志名称
    Loglevel::Level m_level;                   //日志等级
    std::vector<LogAppender::Ptr> m_appenders; //日志输出目的地向量
    LogFormatter::Ptr m_formatter;
};

class LoggerWrapper
{
  public:
    LoggerWrapper(Logger::Ptr logger, LogEvent::Ptr event);
    ~LoggerWrapper();

    std::stringstream &getMessageStream();
    LogEvent::Ptr getEvent() const;

  private:
    Logger::Ptr m_logger;
    LogEvent::Ptr m_event;
};

class LoggerManager
{
  public:
    LoggerManager(const Logger::Ptr logger_root = nullptr);
    ~LoggerManager() = default;

    void setLogger(const std::string &name, const Logger::Ptr logger);
    Logger::Ptr getLogger(const std::string &name);
    Logger::Ptr getRoot();

  private:
    std::map<std::string, Logger::Ptr> m_loggers;
    Logger::Ptr m_logger_root;
};

} // namespace log
} // namespace lon