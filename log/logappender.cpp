#include "log/logappender.h"
#include "logappender.h"

namespace lon
{
namespace log
{
void log::StdoutLogAppender::log(Loglevel::Level level, LogEvent::Ptr event)
{
    if (level < m_level)
    {
        return;
    }
    // std::cout <<
}

FileLogAppender::FileLogAppender(const std::string &filename) : LogAppender(), m_filename(filename)
{
}

void FileLogAppender::log(Loglevel::Level level, LogEvent::Ptr event)

{
    if (level < m_level)
    {
        return;
    }
}
} // namespace log
} // namespace lon