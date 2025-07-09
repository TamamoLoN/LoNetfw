#include "log/logappender.h"
#include "logappender.h"

namespace lon
{
namespace log
{
void LogAppender::setFormatter(LogFormatter::Ptr formatter) { m_formatter = formatter; }

LogFormatter::Ptr LogAppender::getFormatter() const { return m_formatter; }

void StdoutLogAppender::log(std::string logger_name, Loglevel::Level level, LogEvent::Ptr event)
{
    if (level < m_level)
    {
        return;
    }
    std::cout << m_formatter->format(logger_name, level, event);
}

FileLogAppender::FileLogAppender(const std::string &filename) : LogAppender(), m_filename(filename)
{
}

void FileLogAppender::log(std::string logger_name, Loglevel::Level level, LogEvent::Ptr event)
{
    if (level < m_level)
    {
        return;
    }
    if (!m_file.is_open())
    {
        return;
    }
    m_file << m_formatter->format(logger_name, level, event);
}

} // namespace log
} // namespace lon