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
        std::string log_path = "~/.log/tmp.log");
    bool operator==(const ConfigLogAppender &other) const;
    uint8_t type; // 0:StdoutLogAppender 1:FileLogAppender
    log::LogLevel::Level level;
    std::string format;
    std::string log_path;
};

struct ConfigLog
{
    explicit ConfigLog(std::string name                         = "root",
                       log::LogLevel::Level level               = log::LogLevel::Level::DEBUG,
                       std::vector<ConfigLogAppender> appenders = {});
    bool operator==(const ConfigLog &other) const;
    bool operator<(const ConfigLog &other) const;
    std::string name;
    log::LogLevel::Level level;
    std::vector<ConfigLogAppender> appenders;
};

struct GlobalConfig
{
    explicit GlobalConfig();
    static GlobalConfig &Instance();
    ConfigData<std::set<ConfigLog>>::Ptr config_log;
    ConfigData<size_t>::Ptr config_fiber;
    ConfigData<uint32_t>::Ptr config_tcp_timeout;
    ConfigData<uint32_t>::Ptr config_tcp_server_client_timeout;
    ConfigData<uint32_t>::Ptr config_system_daemon_restart_delay_s;
};

//全局变量，使其在main函数之前初始化
// static ConfigLogChanged __log_changed;//这样写会被初始化多次
// static auto global_config = GlobalConfig::Instance();

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