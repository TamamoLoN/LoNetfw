#include "log/logger.h"
#include "logger.h"

namespace lon
{
namespace log
{
Logger::Logger(const std::string &name, LogLevel::Level level)
    : m_name(name), m_appenders({}), m_level(level)
{
    m_formatter = std::make_shared<LogFormatter>(
        "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n");
}

void Logger::log(LogLevel::Level level, LogEvent::Ptr event)
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

void Logger::debug(LogEvent::Ptr event) { log(LogLevel::Level::DEBUG, event); }

void Logger::info(LogEvent::Ptr event) { log(LogLevel::Level::INFO, event); }

void Logger::warn(LogEvent::Ptr event) { log(LogLevel::Level::WARN, event); }

void Logger::error(LogEvent::Ptr event) { log(LogLevel::Level::ERROR, event); }

void Logger::fatal(LogEvent::Ptr event) { log(LogLevel::Level::FATAL, event); }

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

void Logger::setLevel(LogLevel::Level level) { m_level = level; }

void Logger::setLevel(const std::string &level) { m_level = LogLevel::getLevelByName(level); }

LogLevel::Level Logger::getLevel() const { return m_level; }

void Logger::getLevel(std::string &level) { level = LogLevel::getLevelName(m_level); }

std::string Logger::getName() const { return m_name; }

void Logger::test()
{
    std::cout << util::toUpper("ASDASD123Sas") << std::endl;
    if (LogLevel::getLevelByName("ERrOR") == LogLevel::Level::ERROR)
    {
        std::cout << LogLevel::getLevelName(LogLevel::WARN) << std::endl;
    }
}

LoggerWrapper::LoggerWrapper(Logger::Ptr logger, LogEvent::Ptr event)
    : m_logger(logger), m_event(event)
{
}

LoggerWrapper::~LoggerWrapper() { m_logger->log(m_event->getLevel(), m_event); }

std::stringstream &LoggerWrapper::getMessageStream() { return m_event->getMessageStream(); }

LogEvent::Ptr LoggerWrapper::getEvent() const { return m_event; }

LoggerManager::LoggerManager(const Logger::Ptr logger_root) : m_logger_root(logger_root)
{
    if (m_logger_root == nullptr)
    {
        m_logger_root = std::make_shared<Logger>();
        m_logger_root->addAppender(std::make_shared<StdoutLogAppender>());
        m_loggers[m_logger_root->getName()] = m_logger_root;
        return;
    }
    m_loggers[logger_root->getName()] = logger_root;
}

void LoggerManager::setLogger(const std::string &name, const Logger::Ptr logger)
{
    m_loggers[name] = logger;
}

Logger::Ptr LoggerManager::getLogger(const std::string &name)
{
    if (m_loggers.find(name) == m_loggers.end())
    {
        return nullptr;
    }
    return m_loggers[name];
}

Logger::Ptr LoggerManager::getRoot() { return m_logger_root; }

} // namespace log
} // namespace lon