/**
 * 日志格式项
 */
#pragma once
#include "log/logevent.h"
#include <memory>
namespace lon
{
namespace log
{
class Logger;
class LogFormatItem
{
  public:
    using Ptr = std::shared_ptr<LogFormatItem>;
    explicit LogFormatItem(const std::string &str = "") : m_str(str){};
    virtual ~LogFormatItem()                 = default;
    virtual void format(std::ostream &os, std::string logger_name, Loglevel::Level level,
                        LogEvent::Ptr event) = 0;

  protected:
    std::string m_str;
};

class LevelLogFormatItem : public LogFormatItem
{
  public:
    explicit LevelLogFormatItem(const std::string &str) : LogFormatItem(str) {}
    void format(std::ostream &os, std::string logger_name, Loglevel::Level level,
                LogEvent::Ptr event) override;
};

class FilenameLogFormatItem : public LogFormatItem
{
  public:
    explicit FilenameLogFormatItem(const std::string &str) : LogFormatItem(str) {}
    void format(std::ostream &os, std::string logger_name, Loglevel::Level level,
                LogEvent::Ptr event) override;
};

class LineLogFormatItem : public LogFormatItem
{
  public:
    explicit LineLogFormatItem(const std::string &str) : LogFormatItem(str) {}
    void format(std::ostream &os, std::string logger_name, Loglevel::Level level,
                LogEvent::Ptr event) override;
};

class NameLogFormatItem : public LogFormatItem
{
  public:
    explicit NameLogFormatItem(const std::string &str) : LogFormatItem(str) {}
    void format(std::ostream &os, std::string logger_name, Loglevel::Level level,
                LogEvent::Ptr event) override;
};

class ElapseLogFormatItem : public LogFormatItem
{
  public:
    explicit ElapseLogFormatItem(const std::string &str) : LogFormatItem(str) {}
    void format(std::ostream &os, std::string logger_name, Loglevel::Level level,
                LogEvent::Ptr event) override;
};

class ThreadIdLogFormatItem : public LogFormatItem
{
  public:
    explicit ThreadIdLogFormatItem(const std::string &str) : LogFormatItem(str) {}
    void format(std::ostream &os, std::string logger_name, Loglevel::Level level,
                LogEvent::Ptr event) override;
};

class FiberIdLogFormatItem : public LogFormatItem
{
  public:
    explicit FiberIdLogFormatItem(const std::string &format) : LogFormatItem(format) {}
    void format(std::ostream &os, std::string logger_name, Loglevel::Level level,
                LogEvent::Ptr event) override;
};

class DateTimeLogFormatItem : public LogFormatItem
{
  public:
    explicit DateTimeLogFormatItem(const std::string &str,
                                   const std::string &format = "%Y-%m-%d %H:%M:%S")
        : LogFormatItem(str), m_format(format)
    {
    }
    void format(std::ostream &os, std::string logger_name, Loglevel::Level level,
                LogEvent::Ptr event) override;

  private:
    std::string m_format;
};

class MessageLogFormatItem : public LogFormatItem
{
  public:
    explicit MessageLogFormatItem(const std::string &str) : LogFormatItem(str) {}
    void format(std::ostream &os, std::string logger_name, Loglevel::Level level,
                LogEvent::Ptr event) override;
};

class ThreadNameLogFormatItem : public LogFormatItem
{
  public:
    explicit ThreadNameLogFormatItem(const std::string &str) : LogFormatItem(str) {}
    void format(std::ostream &os, std::string logger_name, Loglevel::Level level,
                LogEvent::Ptr event) override;
};

class NewLineLogFormatItem : public LogFormatItem
{
  public:
    explicit NewLineLogFormatItem(const std::string &str = "") : LogFormatItem(str) {}
    void format(std::ostream &os, std::string logger_name, Loglevel::Level level,
                LogEvent::Ptr event) override;
};

class StringLogFormatItem : public LogFormatItem
{
  public:
    explicit StringLogFormatItem(const std::string &str = "") : LogFormatItem(str) {}
    void format(std::ostream &os, std::string logger_name, Loglevel::Level level,
                LogEvent::Ptr event) override;
};

} // namespace log
} // namespace lon