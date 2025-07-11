#include "log/logevent.h"
namespace lon
{
namespace log
{
LogEvent::LogEvent(LogLevel::Level level, const std::string &filename, uint32_t line,
                   uint32_t elapse, uint32_t thread_id, uint32_t fiber_id, uint64_t time,
                   std::string thread_name)
    : m_level(level), m_file(filename), m_line(line), m_elapse(elapse), m_thread_id(thread_id),
      m_fiber_id(fiber_id), m_time(time), m_thread_name(thread_name)
{
}

LogLevel::Level LogEvent::getLevel() const { return m_level; }

std::string LogEvent::getFile() const { return m_file; }

uint32_t LogEvent::getLine() const { return m_line; }

uint32_t LogEvent::getElapse() const { return m_elapse; }

uint32_t LogEvent::getThreadId() const { return m_thread_id; }

uint32_t LogEvent::getFiberId() const { return m_fiber_id; }

uint64_t LogEvent::getTime() const { return m_time; }

std::string LogEvent::getMessage() const { return m_ss.str(); }

std::stringstream &LogEvent::getMessageStream() { return m_ss; }

std::string LogEvent::getThreadName() const { return m_thread_name; }

void LogEvent::setMessageStream(const char *fmt, ...)
{
    va_list al;
    va_start(al, fmt);
    setMessageStream(fmt, al);
    va_end(al);
}

void LogEvent::setMessageStream(const char *fmt, va_list al)
{
    char *buf = nullptr;
    int len   = vasprintf(&buf, fmt, al);
    if (len != -1)
    {
        m_ss << std::string(buf, len);
        free(buf);
    }
}

} // namespace log
} // namespace lon