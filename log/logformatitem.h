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
class LON_API LogFormatItem
{
  public:
    using Ptr = std::shared_ptr<LogFormatItem>;
    explicit LogFormatItem(const std::string &str = "") : m_str(str){};
    virtual ~LogFormatItem()                 = default;
    virtual void format(std::ostream &os, std::string logger_name, LogLevel::Level level,
                        LogEvent::Ptr event) = 0;

  protected:
    std::string m_str;
};

//日志等级输出
class LON_API LevelLogFormatItem : public LogFormatItem
{
  public:
    explicit LevelLogFormatItem(const std::string &str) : LogFormatItem(str) {}
    void format(std::ostream &os, std::string logger_name, LogLevel::Level level,
                LogEvent::Ptr event) override;
};

//文件名输出
class LON_API FilenameLogFormatItem : public LogFormatItem
{
  public:
    explicit FilenameLogFormatItem(const std::string &str) : LogFormatItem(str) {}
    void format(std::ostream &os, std::string logger_name, LogLevel::Level level,
                LogEvent::Ptr event) override;
};

//行号输出
class LON_API LineLogFormatItem : public LogFormatItem
{
  public:
    explicit LineLogFormatItem(const std::string &str) : LogFormatItem(str) {}
    void format(std::ostream &os, std::string logger_name, LogLevel::Level level,
                LogEvent::Ptr event) override;
};

//日志名输出
class LON_API NameLogFormatItem : public LogFormatItem
{
  public:
    explicit NameLogFormatItem(const std::string &str) : LogFormatItem(str) {}
    void format(std::ostream &os, std::string logger_name, LogLevel::Level level,
                LogEvent::Ptr event) override;
};

//持续时间输出
class LON_API ElapseLogFormatItem : public LogFormatItem
{
  public:
    explicit ElapseLogFormatItem(const std::string &str) : LogFormatItem(str) {}
    void format(std::ostream &os, std::string logger_name, LogLevel::Level level,
                LogEvent::Ptr event) override;
};

//线程id输出
class LON_API ThreadIdLogFormatItem : public LogFormatItem
{
  public:
    explicit ThreadIdLogFormatItem(const std::string &str) : LogFormatItem(str) {}
    void format(std::ostream &os, std::string logger_name, LogLevel::Level level,
                LogEvent::Ptr event) override;
};

//协程ID输出
class LON_API FiberIdLogFormatItem : public LogFormatItem
{
  public:
    explicit FiberIdLogFormatItem(const std::string &format) : LogFormatItem(format) {}
    void format(std::ostream &os, std::string logger_name, LogLevel::Level level,
                LogEvent::Ptr event) override;
};

//日期时间输出
class LON_API DateTimeLogFormatItem : public LogFormatItem
{
  public:
    explicit DateTimeLogFormatItem(const std::string &str,
                                   const std::string &format = "%Y-%m-%d %H:%M:%S")
        : LogFormatItem(str), m_format(format)
    {
    }
    void format(std::ostream &os, std::string logger_name, LogLevel::Level level,
                LogEvent::Ptr event) override;

  private:
    std::string m_format;
};

//日志消息输出
class LON_API MessageLogFormatItem : public LogFormatItem
{
  public:
    explicit MessageLogFormatItem(const std::string &str) : LogFormatItem(str) {}
    void format(std::ostream &os, std::string logger_name, LogLevel::Level level,
                LogEvent::Ptr event) override;
};

//线程名输出
class LON_API ThreadNameLogFormatItem : public LogFormatItem
{
  public:
    explicit ThreadNameLogFormatItem(const std::string &str) : LogFormatItem(str) {}
    void format(std::ostream &os, std::string logger_name, LogLevel::Level level,
                LogEvent::Ptr event) override;
};

//换行符输出
class LON_API NewLineLogFormatItem : public LogFormatItem
{
  public:
    explicit NewLineLogFormatItem(const std::string &str = "") : LogFormatItem(str) {}
    void format(std::ostream &os, std::string logger_name, LogLevel::Level level,
                LogEvent::Ptr event) override;
};

//字符串输出
class LON_API StringLogFormatItem : public LogFormatItem
{
  public:
    explicit StringLogFormatItem(const std::string &str = "") : LogFormatItem(str) {}
    void format(std::ostream &os, std::string logger_name, LogLevel::Level level,
                LogEvent::Ptr event) override;
};

//制表符输出
class LON_API TabLogFormatItem : public LogFormatItem
{
  public:
    explicit TabLogFormatItem(const std::string &str = "") : LogFormatItem(str) {}
    void format(std::ostream &os, std::string logger_name, LogLevel::Level level,
                LogEvent::Ptr event) override;
};

} // namespace log
} // namespace lon