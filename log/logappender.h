#pragma once
#include "log/logevent.h"
#include "log/logformatter.h"
#include "log/loglevel.h"
#include "thread/thread.h"
#include <fstream>
#include <memory>
#include <sstream>
namespace lon
{
namespace log
{
class LogAppender
{
  public:
    using Ptr       = std::shared_ptr<LogAppender>;
    using MutexType = thread::SpinLock;
    explicit LogAppender(LogLevel::Level level);
    virtual ~LogAppender()                                                                = default;
    virtual void log(std::string logger_name, LogLevel::Level level, LogEvent::Ptr event) = 0;
    void setFormatter(LogFormatter::Ptr formatter);
    LogFormatter::Ptr getFormatter();
    virtual std::string getYaml() = 0;

  protected:
    LogLevel::Level m_level;
    LogFormatter::Ptr m_formatter;
    MutexType m_mutex;
};

class StdoutLogAppender : public LogAppender
{
  public:
    using Ptr = std::shared_ptr<StdoutLogAppender>;
    explicit StdoutLogAppender(LogLevel::Level level = LogLevel::Level::DEBUG);
    ~StdoutLogAppender() = default;
    void log(std::string logger_name, LogLevel::Level level, LogEvent::Ptr event) override;
    std::string getYaml() override;
};

class FileLogAppender : public LogAppender
{
  public:
    using Ptr = std::shared_ptr<FileLogAppender>;
    explicit FileLogAppender(const std::string &filename,
                             LogLevel::Level level = LogLevel::Level::DEBUG);
    ~FileLogAppender();
    void log(std::string logger_name, LogLevel::Level level, LogEvent::Ptr event) override;
    std::string getYaml() override;

  private:
    std::string m_filename;
    std::ofstream m_file;
};

} // namespace log
} // namespace lon