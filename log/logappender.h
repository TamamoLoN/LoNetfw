#pragma once
#include "log/logevent.h"
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
    using Ptr                                                    = std::shared_ptr<LogAppender>;
    explicit LogAppender()                                       = default;
    virtual ~LogAppender()                                       = default;
    virtual void log(Loglevel::Level level, LogEvent::Ptr event) = 0;

  protected:
    Loglevel::Level m_level;
};

class StdoutLogAppender : public LogAppender
{
  public:
    using Ptr                    = std::shared_ptr<StdoutLogAppender>;
    explicit StdoutLogAppender() = default;
    ~StdoutLogAppender()         = default;
    void log(Loglevel::Level level, LogEvent::Ptr event) override;
};

class FileLogAppender : public LogAppender
{
  public:
    using Ptr = std::shared_ptr<FileLogAppender>;
    explicit FileLogAppender(const std::string &filename);
    ~FileLogAppender() = default;
    void log(Loglevel::Level level, LogEvent::Ptr event) override;

  private:
    std::string m_filename;
};

} // namespace log
} // namespace lon