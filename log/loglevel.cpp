#include "log/loglevel.h"
namespace lon
{
namespace log
{
const std::string LogLevel::getLevelName(LogLevel::Level level)
{
#define GET_STR(t)                                                                                 \
    case t:                                                                                        \
        return #t;                                                                                 \
        break;
    switch (level)
    {
        GET_STR(DEBUG)
        GET_STR(INFO)
        GET_STR(WARN)
        GET_STR(ERROR)
        GET_STR(FATAL)
    default:
        break;
    }
#undef GET_STR
    return "UNKNOWN";
}
LogLevel::Level LogLevel::getLevelByName(const std::string level_name)
{
#define GET_LEVEL(t, v)                                                                            \
    if (util::toUpper(level_name) == #v)                                                           \
        return LogLevel::Level::t;
    GET_LEVEL(DEBUG, DEBUG)
    GET_LEVEL(INFO, INFO)
    GET_LEVEL(WARN, WARN)
    GET_LEVEL(ERROR, ERROR)
    GET_LEVEL(FATAL, FATAL)
#undef GET_LEVEL
    return LogLevel::Level::UNKNOWN;
}
} // namespace log
} // namespace lon