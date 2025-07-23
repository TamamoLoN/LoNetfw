#pragma once

#include "config/config.h"
#include "fiber/fiber.h"
#include "log/logger.h"
#include "thread/thread.h"

namespace lon
{
namespace scheduler
{
class Scheduler
{
  public:
    using Ptr       = std::shared_ptr<Scheduler>;
    using MutexType = thread::Mutex;
    explicit Scheduler(size_t threads_count = 1, bool use_caller = true, std::string name = "");
    virtual ~Scheduler();

    std::string getName() const;

    void start();
    void stop();

    //单次放入
    template <typename FiberOrCB> void schedule(FiberOrCB fc, int thread_id = -1)
    {
        bool should_notify = false;
        {
            MutexType::Lock lock(m_mutex);
            should_notify = mSchedule(fc, thread_id);
        }
        if (should_notify)
        {
            notify();
        }
    }

    //批量放入
    template <typename Iterator> void schedule(Iterator begin, Iterator end)
    {
        bool should_notify = false;
        {
            MutexType::Lock lock(m_mutex);
            for (auto it = begin; it != end; ++it)
            {
                should_notify = mSchedule(&*it) || should_notify;
            }
        }
        if (should_notify)
        {
            notify();
        }
    }

  public:
    static Scheduler *getThis();
    static fiber::Fiber *getMainFiber();

  protected:
    virtual void notify();
    virtual void run();
    virtual bool stopping();
    virtual void idle();
    void setThis();

  private:
    struct TFWrapper
    {
        TFWrapper();
        TFWrapper(fiber::Fiber::Ptr f, int t);
        TFWrapper(fiber::Fiber::Ptr *f, int t); //使传入的智能指针f变为空指针
        TFWrapper(std::function<void()> f, int t);
        TFWrapper(std::function<void()> *f, int t);
        void reset();
        std::function<void()> cb;
        fiber::Fiber::Ptr fiber;
        int thread_id;
    };

  private:
    template <typename FiberOrCB> bool mSchedule(FiberOrCB fc, int thread_id)
    {
        bool should_notify = m_fibers.empty();
        TFWrapper tf(fc, thread_id);
        if (tf.cb || tf.fiber)
        {
            m_fibers.push_back(tf);
        }
        return should_notify;
    }

  private:
    std::vector<thread::Thread::Ptr> m_threads;
    std::vector<TFWrapper> m_fibers;
    fiber::Fiber::Ptr m_root_fiber;
    mutable MutexType m_mutex;
    bool m_use_caller;
    std::string m_name;

  protected:
    std::vector<int> m_threads_id;
    size_t m_threads_count;
    std::atomic<size_t> m_active_threads_count;
    std::atomic<size_t> m_idle_threads_count;
    bool m_stopping;
    bool m_auto_stop;
    int m_root_thread_id;
};
} // namespace scheduler
} // namespace lon