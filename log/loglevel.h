#pragma once
#include "util/util.h"
#include <iostream>
namespace lon
{
namespace log
{
class Loglevel
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

    static const std::string getLevelName(Loglevel::Level level);
    static Loglevel::Level getLevelByName(const std::string level_name);
};
} // namespace log
} // namespace lon