#pragma once

//设计思想：局部变量，出了if语句自动析构
#define LON_LOG_LEVEL(logger, level)                                                               \
    if (logger != nullptr)                                                                         \
        if (logger->getLevel() <= level)                                                           \
    lon::log::LoggerWrapper(logger, std::make_shared<lon::log::LogEvent>(                          \
                                        level, std::string(__FILE__), __LINE__, 0,                 \
                                        lon::util::getThreadId(), lon::fiber::Fiber::getFiberId(), \
                                        lon::util::getCurrentDateTime(),                           \
                                        lon::thread::Thread::getNameStatic()))                     \
        .getMessageStream()
#define LON_DEBUG(logger) LON_LOG_LEVEL(logger, lon::log::LogLevel::Level::DEBUG)
#define LON_INFO(logger) LON_LOG_LEVEL(logger, lon::log::LogLevel::Level::INFO)
#define LON_WARN(logger) LON_LOG_LEVEL(logger, lon::log::LogLevel::Level::WARN)
#define LON_ERROR(logger) LON_LOG_LEVEL(logger, lon::log::LogLevel::Level::ERROR)
#define LON_FATAL(logger) LON_LOG_LEVEL(logger, lon::log::LogLevel::Level::FATAL)

#define LON_LOG_LEVEL_FMT(logger, level, fmt, ...)                                                 \
    if (logger != nullptr)                                                                         \
        if (logger->getLevel() <= level)                                                           \
    lon::log::LoggerWrapper(logger, std::make_shared<lon::log::LogEvent>(                          \
                                        level, std::string(__FILE__), __LINE__, 0,                 \
                                        lon::util::getThreadId(), lon::fiber::Fiber::getFiberId(), \
                                        lon::util::getCurrentDateTime(),                           \
                                        lon::thread::Thread::getNameStatic()))                     \
        .getEvent()                                                                                \
        ->setMessageStream(fmt, __VA_ARGS__)

#define LON_DEBUG_FMT(logger, fmt, ...)                                                            \
    LON_LOG_LEVEL_FMT(logger, lon::log::LogLevel::Level::DEBUG, fmt, __VA_ARGS__)
#define LON_INFO_FMT(logger, fmt, ...)                                                             \
    LON_LOG_LEVEL_FMT(logger, lon::log::LogLevel::Level::INFO, fmt, __VA_ARGS__)
#define LON_WARN_FMT(logger, fmt, ...)                                                             \
    LON_LOG_LEVEL_FMT(logger, lon::log::LogLevel::Level::WARN, fmt, __VA_ARGS__)
#define LON_ERROR_FMT(logger, fmt, ...)                                                            \
    LON_LOG_LEVEL_FMT(logger, lon::log::LogLevel::Level::ERROR, fmt, __VA_ARGS__)
#define LON_FATAL_FMT(logger, fmt, ...)                                                            \
    LON_LOG_LEVEL_FMT(logger, lon::log::LogLevel::Level::FATAL, fmt, __VA_ARGS__)

#ifdef _WIN32
#define LON_LOG_MANAGER lon::log::LoggerManager::Instance()
#else
#define LON_LOG_MANAGER lon::util::Singleton<lon::log::LoggerManager>::Instance()
#endif
#define LON_LOG_ROOT LON_LOG_MANAGER.getRoot()
#define LON_LOG_NAME(name) LON_LOG_MANAGER.getLogger(name, true)

#define LON_ASSERT(arg)                                                                            \
    if (LON_UNLIKELY(!(arg)))                                                                      \
    {                                                                                              \
        LON_ERROR(LON_LOG_ROOT) << "ASSERTATION: " << #arg << "\nbacktrace: \n"                    \
                                << lon::util::backtrace(100, 2, "\t");                             \
        assert(arg);                                                                               \
    }

#define LON_ASSERT_(arg, str)                                                                      \
    if ((LON_UNLIKELY(!(arg))))                                                                    \
    {                                                                                              \
        LON_ERROR(LON_LOG_ROOT) << "ASSERTATION: " << #arg << "\n"                                 \
                                << str << "\nbacktrace: \n"                                        \
                                << lon::util::backtrace(100, 2, "\t");                             \
        assert(arg);                                                                               \
    }
