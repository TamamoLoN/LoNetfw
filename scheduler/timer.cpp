#include "scheduler/timer.h"

namespace lon
{
namespace scheduler
{
Timer::Timer(uint64_t ms, std::function<void()> cb, bool is_loop, TimerManager *manager)
    : m_ms(ms), m_is_loop(is_loop), m_next(util::getCurrentMs() + m_ms), m_manager(manager),
      m_cb(cb)
{
}

Timer::Timer(uint64_t next)
    : m_ms(0), m_is_loop(false), m_next(next), m_manager(nullptr), m_cb(nullptr)
{
}

bool Timer::cancel()
{
    TimerManager::MutexType::WrLock wlock(m_manager->m_mutex);
    if (m_cb)
    {
        m_cb    = nullptr;
        auto it = m_manager->m_timers.find(shared_from_this());
        m_manager->m_timers.erase(it);
        return true;
    }
    return false;
}

bool Timer::refresh()
{
    TimerManager::MutexType::WrLock wlock(m_manager->m_mutex);
    if (!m_cb)
    {
        return false;
    }
    auto it = m_manager->m_timers.find(shared_from_this());
    if (it == m_manager->m_timers.end())
    {
        return false;
    }
    m_manager->m_timers.erase(it);
    m_next = util::getCurrentMs() + m_ms;
    m_manager->m_timers.insert(shared_from_this());
    return true;
}

bool Timer::reset(uint64_t ms, bool from_now)
{
    if (ms == m_ms && !from_now)
    {
        return true;
    }
    TimerManager::MutexType::WrLock wlock(m_manager->m_mutex);
    if (!m_cb)
    {
        return false;
    }

    auto it = m_manager->m_timers.find(shared_from_this());
    if (it == m_manager->m_timers.end())
    {
        return false;
    }
    m_manager->m_timers.erase(it);
    uint64_t start = 0;
    if (from_now)
    {
        start = util::getCurrentMs();
    }
    else
    {
        start = m_next - m_ms;
    }
    m_ms   = ms;
    m_next = start + m_ms;
    m_manager->addTimer(shared_from_this(), wlock);

    return true;
}

bool Timer::Comparator::operator()(const Timer::Ptr &lhs, const Timer::Ptr &rhs) const
{
    if (!lhs && !rhs)
    {
        return false;
    }
    if (!lhs)
    {
        return true;
    }
    if (!rhs)
    {
        return false;
    }
    if (lhs->m_next < rhs->m_next)
    {
        return true;
    }
    if (lhs->m_next > rhs->m_next)
    {
        return false;
    }
    return lhs.get() < rhs.get();
}

TimerManager::TimerManager() : m_notified(false), m_previous_time(util::getCurrentMs()) {}

TimerManager::~TimerManager() {}

Timer::Ptr TimerManager::addTimer(uint64_t ms, std::function<void()> cb, bool is_loop)
{
    Timer::Ptr timer(new Timer(ms, cb, is_loop, this));
    MutexType::WrLock wlock(m_mutex);
    addTimer(timer, wlock);

    return timer;
}

void TimerManager::addTimer(const Timer::Ptr &timer, MutexType::WrLock &lock)
{
    auto it       = m_timers.insert(timer).first;
    bool is_front = (it == m_timers.begin() && !m_notified);
    if (is_front)
    {
        m_notified = true;
    }

    lock.unlock();
    if (is_front)
    {
        onTimerInsertAtFront();
    }
}

Timer::Ptr TimerManager::addConditionTimer(uint64_t ms, std::function<void()> cb,
                                           std::weak_ptr<void> weak_cond, bool is_loop)
{
    auto on_timer = [](std::weak_ptr<void> wc, std::function<void()> c) -> void {
        std::shared_ptr<void> tmp = wc.lock();
        if (tmp)
        {
            c();
        }
    };
    return addTimer(ms, std::bind(on_timer, weak_cond, cb), is_loop);
}

uint64_t TimerManager::getNextTimerTimeMs()
{
    MutexType::RdLock rlock(m_mutex);
    m_notified = false;
    if (m_timers.empty())
    {
        return ~0ull;
    }

    const Timer::Ptr &next_timer = *m_timers.begin();
    uint64_t now_ms              = util::getCurrentMs();
    if (now_ms >= next_timer->m_next)
    {
        return 0;
    }
    else
    {
        return next_timer->m_next - now_ms;
    }
}

void TimerManager::getExpiredCbsList(std::vector<std::function<void()>> &cbs)
{
    uint64_t now_ms                        = util::getCurrentMs();
    std::vector<Timer::Ptr> expired_timers = {};
    {
        MutexType::RdLock rlock(m_mutex);
        if (m_timers.empty())
        {
            return;
        }
    }
    MutexType::WrLock wlock(m_mutex);
    if (m_timers.empty())
    {
        return;
    }

    bool is_modified = detectTimeModified(now_ms);
    if (!is_modified && ((*m_timers.begin())->m_next > now_ms))
    {
        return;
    }

    Timer::Ptr now_timer(new Timer(now_ms));
    auto it = is_modified ? m_timers.end() : m_timers.lower_bound(now_timer);
    while (it != m_timers.end() && (*it)->m_next == now_ms)
    {
        ++it;
    }
    expired_timers.insert(expired_timers.begin(), m_timers.begin(), it);
    m_timers.erase(m_timers.begin(), it);
    cbs.reserve(expired_timers.size());
    for (const auto &timer : expired_timers)
    {
        cbs.push_back(timer->m_cb);
        if (timer->m_is_loop)
        {
            timer->m_next = now_ms + timer->m_ms;
            m_timers.insert(timer);
        }
        else
        {
            timer->m_cb = nullptr;
        }
    }
}

bool TimerManager::hasTimer()
{
    MutexType::RdLock rlock(m_mutex);
    return !m_timers.empty();
}

bool TimerManager::detectTimeModified(uint64_t now_ms)
{
    bool is_modified = false;
    if (now_ms < m_previous_time && now_ms < (m_previous_time - 60 * 60 * 1000))
    {
        is_modified = true;
    }
    m_previous_time = now_ms;
    return is_modified;
}

} // namespace scheduler
} // namespace lon