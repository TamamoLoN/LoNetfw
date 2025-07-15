#pragma once

#include "config/config.h"
#include "log/logger.h"

namespace lon
{
namespace config
{
struct ConfigLogAppender
{
    explicit ConfigLogAppender(
        uint8_t type = 0, log::LogLevel::Level level = log::LogLevel::Level::DEBUG,
        std::string format   = "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n",
        std::string log_path = "~/.log/tmp.log")
        : type(type), level(level), format(format), log_path(log_path)
    {
    }
    bool operator==(const ConfigLogAppender &other) const
    {
        return type == other.type && level == other.level && format == other.format &&
               log_path == other.log_path;
    }
    uint8_t type; // 0:StdoutLogAppender 1:FileLogAppender
    log::LogLevel::Level level;
    std::string format;
    std::string log_path;
};

struct ConfigLog
{
    explicit ConfigLog(std::string name                         = "root",
                       log::LogLevel::Level level               = log::LogLevel::Level::DEBUG,
                       std::vector<ConfigLogAppender> appenders = {})
        : name(name), level(level), appenders(appenders)
    {
    }
    bool operator==(const ConfigLog &other) const
    {
        return name == other.name && level == other.level && appenders == other.appenders;
    }
    bool operator<(const ConfigLog &other) const { return name < other.name; }
    std::string name;
    log::LogLevel::Level level;
    std::vector<ConfigLogAppender> appenders;
};

struct ConfigInitter
{
    explicit ConfigInitter()
    {
        config_log = Config::setData("logs", std::set<ConfigLog>({}), "logs config");
        config_log->addConfigDataChangeCB([](const std::set<ConfigLog> &old_data,
                                             const std::set<ConfigLog> &new_data) {
            LON_INFO(LON_LOG_ROOT) << "on config log data changed";
            for (const auto &data : new_data)
            {
                log::Logger::Ptr logger = nullptr;
                auto it                 = old_data.find(data);
                if (it == old_data.end())
                {
                    //有新增的Logger
                    logger = std::make_shared<log::Logger>(data.name, data.level);
                }
                else
                {
                    if (!(*it == data))
                    {
                        //有修改的Logger
                        logger = LON_LOG_NAME(data.name);
                    }
                }
                logger->clearAppenders();
                for (const auto &_appender : data.appenders)
                {
                    if (_appender.type == 0)
                    {
                        auto appender = std::make_shared<log::StdoutLogAppender>(_appender.level);
                        appender->setFormatter(
                            std::make_shared<log::LogFormatter>(_appender.format));
                        logger->addAppender(appender);
                    }
                    else if (_appender.type == 1)
                    {
                        auto appender = std::make_shared<log::FileLogAppender>(_appender.log_path,
                                                                               _appender.level);
                        appender->setFormatter(
                            std::make_shared<log::LogFormatter>(_appender.format));
                        logger->addAppender(appender);
                    }
                }
                LON_LOG_MANAGER.setLogger(data.name, logger);
            }
            for (const auto &data : old_data)
            {
                auto it = new_data.find(data);
                if (it == new_data.end())
                {
                    //有删除的Logger
                    //仅作软删除，防止有其他还在使用导致崩溃
                    // auto logger = LON_LOG_NAME(it->name);
                    // logger->setLevel((log::LogLevel::Level)100);
                    // logger->clearAppenders();
                    LON_LOG_MANAGER.delLogger(it->name);
                }
            }
        });

        config_fiber =
            Config::setData("fiber.stack_size", (size_t)(1024 * 1024), "fiber stack size");
        config_fiber->addConfigDataChangeCB([](const size_t &old_data, const size_t &new_data) {
            LON_INFO(LON_LOG_ROOT) << "on config fiber data changed";
            LON_DEBUG(LON_LOG_ROOT) << "old_data: " << old_data << " new_data: " << new_data;
        });
    }
    static ConfigInitter &Instance()
    {
        static ConfigInitter instance;
        return instance;
    }
    ConfigData<std::set<ConfigLog>>::Ptr config_log;
    ConfigData<size_t>::Ptr config_fiber;
};

//全局变量，使其在main函数之前初始化
// static ConfigLogChanged __log_changed;//这样写会被初始化多次
auto g_conifg_initter = ConfigInitter::Instance();

} // namespace config
template <> class util::LexicalCast<config::ConfigLogAppender, std::string>
{
  public:
    config::ConfigLogAppender operator()(const std::string &source) const
    {
        YAML::Node node = YAML::Load(source);
        config::ConfigLogAppender res;
        std::stringstream ss;
        if (node["format"].IsDefined())
            res.format = node["format"].as<std::string>();
        if (node["level"].IsDefined())
            res.level = log::LogLevel::getLevelByName(node["level"].as<std::string>());
        if (node["log_path"].IsDefined())
            res.log_path = node["log_path"].as<std::string>();
        if (node["type"].IsDefined())
            res.type = node["type"].as<int>();
        return res;
    }
};

template <> class util::LexicalCast<std::string, config::ConfigLogAppender>
{
  public:
    std::string operator()(const config::ConfigLogAppender &source) const
    {
        YAML::Node node;
        node["format"]   = source.format;
        node["level"]    = log::LogLevel::getLevelName(source.level);
        node["log_path"] = source.log_path;
        node["type"]     = source.type;
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template <> class util::LexicalCast<config::ConfigLog, std::string>
{
  public:
    config::ConfigLog operator()(const std::string &source) const
    {
        YAML::Node node = YAML::Load(source);
        config::ConfigLog res;
        std::stringstream ss;
        if (node["appenders"].IsDefined())
            ss << node["appenders"];
        if (node["name"].IsDefined())
            res.name = node["name"].as<std::string>();
        if (node["level"].IsDefined())
            res.level = log::LogLevel::getLevelByName(node["level"].as<std::string>());
        res.appenders =
            LexicalCast<std::vector<config::ConfigLogAppender>, std::string>()(ss.str());
        return res;
    }
};

template <> class util::LexicalCast<std::string, config::ConfigLog>
{
  public:
    std::string operator()(const config::ConfigLog &source) const
    {
        YAML::Node node;
        node["name"]  = source.name;
        node["level"] = log::LogLevel::getLevelName(source.level);
        node["appenders"] =
            LexicalCast<std::string, std::vector<config::ConfigLogAppender>>()(source.appenders);
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};
} // namespace lon