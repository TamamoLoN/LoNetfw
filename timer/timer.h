#pragma once

#include "thread/thread.h"

namespace lon
{
namespace timer
{
class TimerManager;
class Timer : public std::enable_shared_from_this<Timer>
{
    friend class TimerManager;

  public:
    using Ptr = std::shared_ptr<Timer>;
    bool cancel();
    bool refresh();
    bool reset(uint64_t ms, bool from_now);

  private:
    explicit Timer(uint64_t ms, std::function<void()> cb, bool is_loop, TimerManager *manager);
    explicit Timer(uint64_t next);
    // virtual ~Timer() = default;

  private:
    std::function<void()> m_cb;
    bool m_is_loop;  //是否循环定时器
    uint64_t m_ms;   //执行周期
    uint64_t m_next; //下一个执行周期的时间
    TimerManager *m_manager;

  private:
    struct Comparator
    {
        bool operator()(const Timer::Ptr &lhs, const Timer::Ptr &rhs) const;
    };
};

class TimerManager
{
    friend class Timer;

  public:
    using Ptr       = std::shared_ptr<TimerManager>;
    using MutexType = thread::RWMutex;

    explicit TimerManager();
    virtual ~TimerManager();

    Timer::Ptr addTimer(uint64_t ms, std::function<void()> cb, bool is_loop = false);
    Timer::Ptr addConditionTimer(uint64_t ms, std::function<void()> cb,
                                 std::weak_ptr<void> weak_cond, bool is_loop = false);
    uint64_t getNextTimerTimeMs();
    void getExpiredCbsList(std::vector<std::function<void()>> &cbs);
    bool hasTimer();

  protected:
    virtual void onTimerInsertAtFront() = 0;
    void addTimer(const Timer::Ptr &timer, MutexType::WrLock &lock);

  private:
    bool detectTimeModified(uint64_t now_ms);

  private:
    MutexType m_mutex;
    std::set<Timer::Ptr, Timer::Comparator> m_timers;
    bool m_notified;
    uint64_t m_previous_time;
};
} // namespace timer
} // namespace lon