#include "log/logappender.h"
#include "logappender.h"

namespace lon
{
namespace log
{
LogAppender::LogAppender(Loglevel::Level level) : m_level(level) {}

void LogAppender::setFormatter(LogFormatter::Ptr formatter) { m_formatter = formatter; }

LogFormatter::Ptr LogAppender::getFormatter() const { return m_formatter; }

StdoutLogAppender::StdoutLogAppender(Loglevel::Level level) : LogAppender(level) {}

void StdoutLogAppender::log(std::string logger_name, Loglevel::Level level, LogEvent::Ptr event)
{
    if (level < m_level)
    {
        return;
    }
    auto str = m_formatter->format(logger_name, level, event);
    switch (level)
    {
    case Loglevel::Level::INFO:
        util::getColorStr(str, util::Color::GREEN);
        break;
    case Loglevel::Level::WARN:
        util::getColorStr(str, util::Color::YELLOW);
        break;
    case Loglevel::Level::ERROR:
        util::getColorStr(str, util::Color::RED);
        break;
    case Loglevel::Level::FATAL:
        util::getColorStr(str, util::Color::PURPLE);
        break;
    default:
        util::getColorStr(str, util::Color::DEFAULT);
        break;
    }
    std::cout << str;
}

FileLogAppender::FileLogAppender(const std::string &filename, Loglevel::Level level)
    : LogAppender(level), m_filename(filename)
{
    m_file.open(filename, std::ios::app);
}

FileLogAppender::~FileLogAppender()
{
    if (m_file.is_open())
    {
        m_file.close();
    }
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
    m_file.flush();
}

} // namespace log
} // namespace lon