#include "log/logger.h"
#include "logger.h"

namespace lon
{
namespace log
{
static auto g_logger = LON_LOG_ROOT;

Logger::Logger(const std::string &name, LogLevel::Level level)
    : m_name(name), m_appenders({}), m_level(level)
{
    m_formatter = std::make_shared<LogFormatter>(
        "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n");
}

void Logger::log(LogLevel::Level level, LogEvent::Ptr event)
{
    MutexType::Lock lock(m_mutex);
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
    MutexType::Lock lock(m_mutex);
    if (!appender->getFormatter())
    {
        appender->setFormatter(m_formatter);
    }
    m_appenders.push_back(appender);
}

void Logger::delAppender(LogAppender::Ptr appender)
{
    MutexType::Lock lock(m_mutex);
    for (auto it = m_appenders.begin(); it != m_appenders.end(); ++it)
    {
        if (*it == appender)
        {
            m_appenders.erase(it);
            break;
        }
    }
}

void Logger::clearAppenders()
{
    MutexType::Lock lock(m_mutex);
    m_appenders.clear();
}

void Logger::setLevel(LogLevel::Level level) { m_level = level; }

void Logger::setLevel(const std::string &level) { m_level = LogLevel::getLevelByName(level); }

LogLevel::Level Logger::getLevel() const { return m_level; }

void Logger::getLevel(std::string &level) { level = LogLevel::getLevelName(m_level); }

std::string Logger::getName() const { return m_name; }

std::string Logger::getYaml()
{
    YAML::Node root;
    MutexType::Lock lock(m_mutex);
    root["name"]  = m_name;
    root["level"] = LogLevel::getLevelName(m_level);
    auto node     = root["appenders"];
    for (const auto &it : m_appenders)
    {
        node.push_back(YAML::Load(it->getYaml()));
    }
    std::stringstream ss;
    ss << root;
    return ss.str();
}

LoggerWrapper::LoggerWrapper(Logger::Ptr logger, LogEvent::Ptr event)
    : m_logger(logger), m_event(event)
{
}

LoggerWrapper::~LoggerWrapper() { m_logger->log(m_event->getLevel(), m_event); }

std::stringstream &LoggerWrapper::getMessageStream() { return m_event->getMessageStream(); }

LogEvent::Ptr LoggerWrapper::getEvent() const { return m_event; }

LoggerManager::LoggerManager(const Logger::Ptr &logger_root) : m_logger_root(logger_root)
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

void LoggerManager::setLogger(const std::string &name, const Logger::Ptr &logger)
{
    MutexType::Lock lock(m_mutex);
    m_loggers[name] = logger;
}

Logger::Ptr LoggerManager::getLogger(const std::string &name, bool auto_create)
{
    MutexType::Lock lock(m_mutex);
    if (m_loggers.find(name) == m_loggers.end())
    {
        if (auto_create)
        {
            auto new_logger = std::make_shared<Logger>(name);
            new_logger->addAppender(std::make_shared<StdoutLogAppender>(LogLevel::Level::DEBUG));
            m_loggers[name] = new_logger;
        }
        else
        {
            LON_WARN(g_logger) << "the logger has not been initialized: " << name;
            return nullptr;
        }
    }
    return m_loggers[name];
}

void LoggerManager::delLogger(const std::string &name)
{
    MutexType::Lock lock(m_mutex);
    auto it = m_loggers.find(name);
    if (it == m_loggers.end())
    {
        return;
    }
    m_loggers.erase(it);
}

Logger::Ptr LoggerManager::getRoot()
{
    MutexType::Lock lock(m_mutex);
    if (m_loggers.find("root") == m_loggers.end())
    {
        return m_logger_root;
    }
    else
    {
        return m_loggers["root"];
    }
}

std::string LoggerManager::getYaml()
{
    YAML::Node root;
    MutexType::Lock lock(m_mutex);
    auto node = root["logs"];
    for (const auto &it : m_loggers)
    {
        node.push_back(YAML::Load(it.second->getYaml()));
    }
    std::stringstream ss;
    ss << root;
    return ss.str();
}

} // namespace log
} // namespace lon