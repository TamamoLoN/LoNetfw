#include "log/logappender.h"
#include "logappender.h"

namespace lon
{
namespace log
{
LogAppender::LogAppender(LogLevel::Level level) : m_level(level) {}

void LogAppender::setFormatter(LogFormatter::Ptr formatter)
{
    thread::Mutex::Lock lock(m_mutex);
    m_formatter = formatter;
}

LogFormatter::Ptr LogAppender::getFormatter()
{
    thread::Mutex::Lock lock(m_mutex);
    return m_formatter;
}

StdoutLogAppender::StdoutLogAppender(LogLevel::Level level) : LogAppender(level) {}

void StdoutLogAppender::log(std::string logger_name, LogLevel::Level level, LogEvent::Ptr event)
{
    if (level < m_level)
    {
        return;
    }
    thread::Mutex::Lock lock(m_mutex);
    auto str = m_formatter->format(logger_name, level, event);
    switch (level)
    {
    case LogLevel::Level::INFO:
        util::getColorStr(str, util::Color::GREEN);
        break;
    case LogLevel::Level::WARN:
        util::getColorStr(str, util::Color::YELLOW);
        break;
    case LogLevel::Level::ERROR:
        util::getColorStr(str, util::Color::RED);
        break;
    case LogLevel::Level::FATAL:
        util::getColorStr(str, util::Color::PURPLE);
        break;
    default:
        util::getColorStr(str, util::Color::DEFAULT);
        break;
    }
    std::cout << str;
}

std::string StdoutLogAppender::getYaml()
{
    YAML::Node node;
    thread::Mutex::Lock lock(m_mutex);
    node["type"]   = 0;
    node["format"] = m_formatter->getFormat();
    node["level"]  = LogLevel::getLevelName(m_level);
    std::stringstream ss;
    ss << node;
    return ss.str();
}

FileLogAppender::FileLogAppender(const std::string &filename, LogLevel::Level level)
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

void FileLogAppender::log(std::string logger_name, LogLevel::Level level, LogEvent::Ptr event)
{
    if (level < m_level)
    {
        return;
    }
    if (!m_file.is_open())
    {
        return;
    }
    thread::Mutex::Lock lock(m_mutex);
    m_file << m_formatter->format(logger_name, level, event);
    m_file.flush();
}

std::string FileLogAppender::getYaml()
{
    YAML::Node node;
    thread::Mutex::Lock lock(m_mutex);
    node["type"]     = 1;
    node["format"]   = m_formatter->getFormat();
    node["level"]    = LogLevel::getLevelName(m_level);
    node["log_path"] = m_filename;
    std::stringstream ss;
    ss << node;
    return ss.str();
}

} // namespace log
} // namespace lon