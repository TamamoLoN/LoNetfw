#include "log/logformatitem.h"

namespace lon
{
namespace log
{
void LevelLogFormatItem::format(std::ostream &os, std::string logger_name, Loglevel::Level level,
                                LogEvent::Ptr event)
{
    // os << Loglevel::getLevelName(event->getLevel());
    os << Loglevel::getLevelName(level);
}

void FilenameLogFormatItem::format(std::ostream &os, std::string logger_name, Loglevel::Level level,
                                   LogEvent::Ptr event)
{
    os << event->getFile();
}

void LineLogFormatItem::format(std::ostream &os, std::string logger_name, Loglevel::Level level,
                               LogEvent::Ptr event)
{
    os << event->getLine();
}

void NameLogFormatItem::format(std::ostream &os, std::string logger_name, Loglevel::Level level,
                               LogEvent::Ptr event)
{
    os << logger_name;
}

void ElapseLogFormatItem::format(std::ostream &os, std::string logger_name, Loglevel::Level level,
                                 LogEvent::Ptr event)
{
    os << event->getMessage();
}

void ThreadIdLogFormatItem::format(std::ostream &os, std::string logger_name, Loglevel::Level level,
                                   LogEvent::Ptr event)
{
    os << event->getThreadId();
}

void FiberIdLogFormatItem::format(std::ostream &os, std::string logger_name, Loglevel::Level level,
                                  LogEvent::Ptr event)
{
    os << event->getFiberId();
}

void DateTimeLogFormatItem::format(std::ostream &os, std::string logger_name, Loglevel::Level level,
                                   LogEvent::Ptr event)
{
    if (m_format.empty())
    {
        os << event->getTime();
    }
    else
    {
        os << util::getDateTime(event->getTime(), m_format);
    }
}

void MessageLogFormatItem::format(std::ostream &os, std::string logger_name, Loglevel::Level level,
                                  LogEvent::Ptr event)
{
    os << event->getMessage();
}

void ThreadNameLogFormatItem::format(std::ostream &os, std::string logger_name,
                                     Loglevel::Level level, LogEvent::Ptr event)
{
    os << event->getThreadName();
}

void NewLineLogFormatItem::format(std::ostream &os, std::string logger_name, Loglevel::Level level,
                                  LogEvent::Ptr event)
{
    os << std::endl;
}

void StringLogFormatItem::format(std::ostream &os, std::string logger_name, Loglevel::Level level,
                                 LogEvent::Ptr event)
{
    os << m_str;
}

} // namespace log
} // namespace lon
