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

struct ConfigServer
{
    explicit ConfigServer(std::string name               = "LoNetfw" LONETFW_VERSION,
                          std::vector<std::string> addrs = {"0.0.0.0:8080"},
                          uint32_t recv_timeout = 1000, uint32_t send_timeout = 1000,
                          std::string accept_scheduler  = "io_scheduler",
                          std::string process_scheduler = "io_scheduler", std::string type = "tcp",
                          uint8_t ssl = 0, std::string cert_file = "", std::string key_file = "");
    bool operator==(const ConfigServer &other) const;
    bool operator<(const ConfigServer &other) const;
    std::string name;
    std::vector<std::string> addrs;
    uint32_t recv_timeout;
    uint32_t send_timeout;
    std::string accept_scheduler;
    std::string process_scheduler;
    std::string type; // http, ws, tcp ...
    uint8_t ssl;      // 0:no, 1:yes
    std::string cert_file;
    std::string key_file;
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
    ConfigData<std::vector<ConfigServer>>::Ptr config_servers;
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

template <> class util::LexicalCast<config::ConfigServer, std::string>
{
  public:
    config::ConfigServer operator()(const std::string &source) const
    {
        YAML::Node node = YAML::Load(source);
        config::ConfigServer res;
        std::stringstream ss;
        if (node["name"].IsDefined())
            res.name = node["name"].as<std::string>();
        if (node["addrs"].IsDefined())
            ss << node["addrs"];
        if (node["recv_timeout"].IsDefined())
            res.name = node["recv_timeout"].as<uint32_t>();
        if (node["send_timeout"].IsDefined())
            res.name = node["send_timeout"].as<uint32_t>();
        if (node["accept_scheduler"].IsDefined())
            res.name = node["accept_scheduler"].as<std::string>();
        if (node["process_scheduler"].IsDefined())
            res.name = node["process_scheduler"].as<std::string>();
        if (node["type"].IsDefined())
            res.name = node["type"].as<std::string>();
        if (node["ssl"].IsDefined())
            res.name = node["ssl"].as<uint8_t>();
        if (node["cert_file"].IsDefined())
            res.name = node["cert_file"].as<std::string>();
        if (node["key_file"].IsDefined())
            res.name = node["key_file"].as<std::string>();
        res.addrs = LexicalCast<std::vector<std::string>, std::string>()(ss.str());
        return res;
    }
};

template <> class util::LexicalCast<std::string, config::ConfigServer>
{
  public:
    std::string operator()(const config::ConfigServer &source) const
    {
        YAML::Node node;
        node["name"]         = source.name;
        node["addrs"]        = LexicalCast<std::string, std::vector<std::string>>()(source.addrs);
        node["recv_timeout"] = source.recv_timeout;
        node["send_timeout"] = source.send_timeout;
        node["accept_scheduler"]  = source.accept_scheduler;
        node["process_scheduler"] = source.process_scheduler;
        node["type"]              = source.type;
        node["ssl"]               = source.ssl;
        node["cert_file"]         = source.cert_file;
        node["key_file"]          = source.key_file;
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

} // namespace lon