#include "log/logger.h"
#include "logger.h"

namespace lon
{
namespace log
{
Logger::Logger(const std::string &name)
    : m_name(name), m_appenders({}), m_level(Loglevel::Level::DEBUG)
{
    m_formatter = std::make_shared<LogFormatter>("%c->%d [%p] <%f:%l>: %m %n");
}

void Logger::log(Loglevel::Level level, LogEvent::Ptr event)
{
    if (m_level > level)
    {
        return;
    }
    for (const auto &it : m_appenders)
    {
        it->log(shared_from_this()->m_name, level, event);
    }
}

void Logger::debug(LogEvent::Ptr event) { log(Loglevel::Level::DEBUG, event); }

void Logger::info(LogEvent::Ptr event) { log(Loglevel::Level::INFO, event); }

void Logger::warn(LogEvent::Ptr event) { log(Loglevel::Level::WARN, event); }

void Logger::error(LogEvent::Ptr event) { log(Loglevel::Level::ERROR, event); }

void Logger::fatal(LogEvent::Ptr event) { log(Loglevel::Level::FATAL, event); }

void Logger::addAppender(LogAppender::Ptr appender)
{
    if (!appender->getFormatter())
    {
        appender->setFormatter(m_formatter);
    }
    m_appenders.push_back(appender);
}

void Logger::delAppender(LogAppender::Ptr appender)
{
    for (auto it = m_appenders.begin(); it != m_appenders.end(); ++it)
    {
        if (*it == appender)
        {
            m_appenders.erase(it);
            break;
        }
    }
}

void Logger::setLevel(Loglevel::Level level) { m_level = level; }

void Logger::setLevel(const std::string &level) { m_level = Loglevel::getLevelByName(level); }

std::string Logger::getLevel() const { return Loglevel::getLevelName(m_level); }

void Logger::getLevel(Loglevel::Level &level) { level = m_level; }

std::string Logger::getName() const { return m_name; }

void Logger::test()
{
    std::cout << util::toUpper("ASDASD123Sas") << std::endl;
    if (Loglevel::getLevelByName("ERrOR") == Loglevel::Level::ERROR)
    {
        std::cout << Loglevel::getLevelName(Loglevel::WARN) << std::endl;
    }
}
} // namespace log
} // namespace lon