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
    using Ptr       = std::shared_ptr<Logger>;
    using MutexType = thread::SpinLock;
    explicit Logger(const std::string &name = "root",
                    LogLevel::Level level   = LogLevel::Level::DEBUG);
    virtual ~Logger() = default;
    void log(LogLevel::Level level, LogEvent::Ptr event);
    void debug(LogEvent::Ptr event);
    void info(LogEvent::Ptr event);
    void warn(LogEvent::Ptr event);
    void error(LogEvent::Ptr event);
    void fatal(LogEvent::Ptr event);

    void addAppender(LogAppender::Ptr appender);
    void delAppender(LogAppender::Ptr appender);
    void clearAppenders();
    void setLevel(LogLevel::Level level);
    void setLevel(const std::string &level);
    LogLevel::Level getLevel() const;
    void getLevel(std::string &level);
    std::string getName() const;
    std::string getYaml();

  private:
    std::string m_name;                        //日志名称
    LogLevel::Level m_level;                   //日志等级
    std::vector<LogAppender::Ptr> m_appenders; //日志输出目的地向量
    LogFormatter::Ptr m_formatter;
    mutable MutexType m_mutex;
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
    using MutexType = thread::SpinLock;
    LoggerManager(const Logger::Ptr &logger_root = nullptr);
    ~LoggerManager() = default;

    void setLogger(const std::string &name, const Logger::Ptr &logger);
    Logger::Ptr getLogger(const std::string &name);
    void delLogger(const std::string &name);
    Logger::Ptr getRoot();
    std::string getYaml();

  private:
    std::map<std::string, Logger::Ptr> m_loggers;
    Logger::Ptr m_logger_root;
    mutable MutexType m_mutex;
};

} // namespace log
} // namespace lon