#include "config/globalconfig.h"

namespace lon
{
namespace config
{
//全局变量，使其在main函数之前初始化
static auto global_config = GlobalConfig::Instance();

ConfigLogAppender::ConfigLogAppender(uint8_t type, log::LogLevel::Level level, std::string format,
                                     std::string log_path)
    : type(type), level(level), format(format), log_path(log_path)
{
}

bool ConfigLogAppender::operator==(const ConfigLogAppender &other) const
{
    return type == other.type && level == other.level && format == other.format &&
           log_path == other.log_path;
}

ConfigLog::ConfigLog(std::string name, log::LogLevel::Level level,
                     std::vector<ConfigLogAppender> appenders)
    : name(name), level(level), appenders(appenders)
{
}

bool ConfigLog::operator==(const ConfigLog &other) const
{
    return name == other.name && level == other.level && appenders == other.appenders;
}

bool ConfigLog::operator<(const ConfigLog &other) const { return name < other.name; }

GlobalConfig::GlobalConfig()
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
                // logger = std::make_shared<log::Logger>(data.name, data.level);
                logger = LON_LOG_NAME(data.name);
            }
            else
            {
                if (!(*it == data))
                {
                    //有修改的Logger
                    logger = LON_LOG_NAME(data.name);
                }
                else
                {
                    continue;
                }
            }
            logger->setLevel(data.level);
            logger->clearAppenders();
            for (const auto &_appender : data.appenders)
            {
                if (_appender.type == 0)
                {
                    auto appender = std::make_shared<log::StdoutLogAppender>(_appender.level);
                    appender->setFormatter(std::make_shared<log::LogFormatter>(_appender.format));
                    logger->addAppender(appender);
                }
                else if (_appender.type == 1)
                {
                    auto appender =
                        std::make_shared<log::FileLogAppender>(_appender.log_path, _appender.level);
                    appender->setFormatter(std::make_shared<log::LogFormatter>(_appender.format));
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

    config_fiber = Config::setData("fiber.stack_size", (size_t)(1024 * 1024), "fiber stack size");
    config_fiber->addConfigDataChangeCB([](const size_t &old_data, const size_t &new_data) {
        LON_INFO(LON_LOG_ROOT) << "on config fiber data changed";
        LON_DEBUG(LON_LOG_ROOT) << "old_data: " << old_data << " new_data: " << new_data;
    });
    config_tcp_timeout =
        Config::setData("tcp.connect.timeout", (uint32_t)5000, "tcp connect timeout ms");
    config_tcp_timeout->addConfigDataChangeCB([](const size_t &old_data, const size_t &new_data) {
        LON_INFO(LON_LOG_ROOT) << "on config tcp connect timeout data changed";
        LON_DEBUG(LON_LOG_ROOT) << "old_data: " << old_data << " new_data: " << new_data;
    });

    config_tcp_server_client_timeout =
        Config::setData("tcp.server.client_timeout", (uint32_t)(1000 * 60 * 2),
                        "tcp server client timeout ms(default 2min)");
    config_tcp_server_client_timeout->addConfigDataChangeCB(
        [](const size_t &old_data, const size_t &new_data) {
            LON_INFO(LON_LOG_ROOT) << "on config tcp server client timeout data changed";
            LON_DEBUG(LON_LOG_ROOT) << "old_data: " << old_data << " new_data: " << new_data;
        });

    config_system_daemon_restart_delay_s =
        Config::setData("system.daemon.restart_delay_s", (uint32_t)(2),
                        "daemon restart delay if process crash(default 2 sec)");
    config_system_daemon_restart_delay_s->addConfigDataChangeCB(
        [](const size_t &old_data, const size_t &new_data) {
            LON_INFO(LON_LOG_ROOT) << "on system daemon restart delay second data changed";
            LON_DEBUG(LON_LOG_ROOT) << "old_data: " << old_data << " new_data: " << new_data;
        });
}

GlobalConfig &GlobalConfig::Instance()
{
    static GlobalConfig instance;
    return instance;
}

} // namespace config
} // namespace lon
