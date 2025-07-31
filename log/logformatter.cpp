#include "log/logformatter.h"
namespace lon
{
namespace log
{
/**
 *  %m 消息
 *  %p 日志级别
 *  %r 累计毫秒数
 *  %c 日志名称
 *  %t 线程id
 *  %n 换行
 *  %d 时间
 *  %f 文件名
 *  %l 行号
 *  %T 制表符
 *  %F 协程id
 *  %N 线程名称
 */
LogFormatter::LogFormatter(const std::string &format) : m_format(format), m_items({})
{
    m_item_factory = {
#define ITEM_FACTORY(t, v)                                                                         \
    {                                                                                              \
#t, [](const std::string &fmt) { return std::make_shared<v>(fmt); }                        \
    }
        ITEM_FACTORY(m, MessageLogFormatItem),  ITEM_FACTORY(p, LevelLogFormatItem),
        ITEM_FACTORY(r, ElapseLogFormatItem),   ITEM_FACTORY(c, NameLogFormatItem),
        ITEM_FACTORY(t, ThreadIdLogFormatItem), ITEM_FACTORY(l, LineLogFormatItem),
        ITEM_FACTORY(d, DateTimeLogFormatItem), ITEM_FACTORY(f, FilenameLogFormatItem),
        ITEM_FACTORY(F, FiberIdLogFormatItem),  ITEM_FACTORY(N, ThreadNameLogFormatItem),
        ITEM_FACTORY(n, NewLineLogFormatItem),  ITEM_FACTORY(T, TabLogFormatItem),
#undef ITEM_FACTORY
    };
    auto vec = util::formatParser(m_format);
    for (const auto &map : vec)
    {
        for (const auto &it : map)
        {
            if (it.second == 0)
            {
                m_items.push_back(std::make_shared<StringLogFormatItem>(it.first));
            }
            else if (it.second == 1)
            {
                if (m_item_factory.find(it.first) != m_item_factory.end())
                {
                    m_items.push_back(m_item_factory[it.first](it.first));
                }
                else
                {
                    m_items.push_back(std::make_shared<StringLogFormatItem>("<unknown format:%" +
                                                                            it.first + ">"));
                }
            }
            else if (it.second == 2)
            {
                m_items.push_back(std::make_shared<DateTimeLogFormatItem>(it.first, it.first));
            }
        }
    }
}

std::string LogFormatter::format(std::string logger_name, LogLevel::Level level,
                                 LogEvent::Ptr event)
{
    std::stringstream ss;
    for (const auto &it : m_items)
    {
        it->format(ss, logger_name, level, event);
    }
    return ss.str();
}

void LogFormatter::setFormat(const std::string &format) { m_format = format; }

std::string LogFormatter::getFormat() const { return m_format; }

} // namespace log
} // namespace lon