#pragma once
#include "util/util.h"
#include <iostream>
namespace lon
{
namespace log
{
class LogLevel
{
  public:
    enum Level
    {
        UNKNOWN = 0,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL
    };

    static const std::string getLevelName(LogLevel::Level level);
    static LogLevel::Level getLevelByName(const std::string level_name);
};
} // namespace log
} // namespace lon