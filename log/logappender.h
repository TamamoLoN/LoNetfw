#pragma once
#include "log/logevent.h"
#include "log/logformatter.h"
#include "log/loglevel.h"
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
    using Ptr = std::shared_ptr<LogAppender>;
    explicit LogAppender(Loglevel::Level level);
    virtual ~LogAppender()                                                                = default;
    virtual void log(std::string logger_name, Loglevel::Level level, LogEvent::Ptr event) = 0;
    void setFormatter(LogFormatter::Ptr formatter);
    LogFormatter::Ptr getFormatter() const;

  protected:
    Loglevel::Level m_level;
    LogFormatter::Ptr m_formatter;
};

class StdoutLogAppender : public LogAppender
{
  public:
    using Ptr = std::shared_ptr<StdoutLogAppender>;
    explicit StdoutLogAppender(Loglevel::Level level = Loglevel::Level::DEBUG);
    ~StdoutLogAppender() = default;
    void log(std::string logger_name, Loglevel::Level level, LogEvent::Ptr event) override;
};

class FileLogAppender : public LogAppender
{
  public:
    using Ptr = std::shared_ptr<FileLogAppender>;
    explicit FileLogAppender(const std::string &filename,
                             Loglevel::Level level = Loglevel::Level::DEBUG);
    ~FileLogAppender();
    void log(std::string logger_name, Loglevel::Level level, LogEvent::Ptr event) override;

  private:
    std::string m_filename;
    std::ofstream m_file;
};

} // namespace log
} // namespace lon