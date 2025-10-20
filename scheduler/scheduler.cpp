#include "scheduler/scheduler.h"

namespace lon
{
namespace scheduler
{
// 当前协程调度器
static thread_local Scheduler *t_cur_scheduler = nullptr;
// 调度器协程
static thread_local fiber::Fiber *t_scheduler_fiber = nullptr;

Scheduler::Scheduler(size_t threads_count, bool use_caller, std::string name,
                     size_t fiber_stack_size)
    : m_threads_count(threads_count), m_active_threads_count(0), m_idle_threads_count(0),
      m_root_thread_id(-1), m_stopping(true), m_auto_stop(false), m_use_caller(use_caller),
      m_name(name), m_threads({}), m_fiber_stack_size(fiber_stack_size)
{
    LON_ASSERT(m_threads_count > 0);
    if (m_use_caller)
    {
        fiber::Fiber::getThis();
        --m_threads_count;
        LON_ASSERT(getThis() == nullptr);
        t_cur_scheduler = this;
        m_root_fiber.reset(new fiber::Fiber(std::bind(&Scheduler::run, this), m_fiber_stack_size));
        thread::Thread::setNameStatic(m_name);
        // 设置当前调度器协程为m_root_fiber
        // 这里的m_root_fiber是该调度器协程并非线程主协程（执行run任务的协程），只有默认构造出来的fiber才是主协程
        t_scheduler_fiber = m_root_fiber.get();
        m_root_thread_id  = util::getThreadId();
        m_threads_id.push_back(m_root_thread_id);
    }
}

Scheduler::~Scheduler()
{
    LON_ASSERT_(m_stopping, "schedule never stop!");
    if (getThis() == this)
    {
        t_cur_scheduler = nullptr;
    }
}

std::string Scheduler::getName() const { return m_name; }

void Scheduler::start()
{
    MutexType::Lock lock(m_mutex);
    if (!m_stopping)
    {
        return;
    }
    m_stopping = false;
    LON_ASSERT(m_threads.empty());
    m_threads.resize(m_threads_count);
    for (int cnt = 0; cnt < m_threads_count; ++cnt)
    {
        m_threads[cnt].reset(new thread::Thread(
            std::bind(&Scheduler::run, this), m_name + "_" + util::lexical_cast<std::string>(cnt)));
        m_threads_id.push_back(m_threads[cnt]->getId());
    }
    /*
     * 在这里切换线程时，swap的话会将线程的主协程与当前协程交换，当使用use_caller时，t_fiber =
     * m_root_fiber，swapIn是将当前协程与主协程交换
     * 为了确保在启动之后仍有任务加入任务队列中，所以在stop()中做该线程的启动，这样就不会漏掉任务队列中的任务
     */
    // lock.unlock();
    // if (m_root_fiber)
    // {
    //     m_root_fiber->swapIn();
    // }
    LON_INFO(LON_LOG_ROOT) << "scheduler[" << m_name << "]:" << this << " started";
}

void Scheduler::stop()
{
    // MutexType::Lock lock(m_mutex);
    m_auto_stop = true;
    // 使用use_caller,并且只有一个线程，并且主协程的状态为结束或者初始化
    if (m_root_fiber && m_threads_count == 0 &&
        (m_root_fiber->getState() == fiber::Fiber::TERM ||
         m_root_fiber->getState() == fiber::Fiber::INIT))
    {
        LON_INFO(LON_LOG_ROOT) << "scheduler[" << m_name << "]:" << this << " stopped";
        m_stopping = true;
        if (stopping())
        {
            return;
        }
    }

    // bool exit_on_this_fiber = false;
    // use_caller线程, 当前调度器和t_secheduler相同
    if (m_root_thread_id != -1)
    {
        LON_ASSERT(this == getThis());
    }
    // 非use_caller，此时的t_secheduler应该为nullptr
    else
    {
        LON_ASSERT(this != getThis());
    }
    m_stopping = true;
    for (size_t cnt = 0; cnt < m_threads_count; ++cnt)
    {
        notify();
    }
    if (m_root_fiber)
    {
        notify();
    }
    if (m_root_fiber)
    {
        if (!stopping())
        {
            m_root_fiber->swapIn();
        }
    }

    std::vector<thread::Thread::Ptr> threads;
    {
        MutexType::Lock lock(m_mutex);
        threads.swap(m_threads);
    }
    for (const auto &it : threads)
    {
        it->join();
    }
}

void Scheduler::notify() { LON_DEBUG(LON_LOG_ROOT) << "notify"; }

void Scheduler::run()
{
    util::HookState::enable();
    setThis();
    // 非user_caller线程，设置调度器协程为线程主协程
    if (util::getThreadId() != m_root_thread_id)
    {
        t_scheduler_fiber = fiber::Fiber::getThis().get();
    }
    fiber::Fiber::Ptr idle_fiber(
        new fiber::Fiber(std::bind(&Scheduler::idle, this), m_fiber_stack_size));

    fiber::Fiber::Ptr cb_fiber;
    Task task;

    while (true)
    {
        task.reset();
        bool is_active     = false;
        bool should_notify = false;
        {
            MutexType::Lock lock(m_mutex);
            auto it = m_tasks.begin();
            while (it != m_tasks.end())
            {
                if (it->thread_id != -1 && it->thread_id != util::getThreadId())
                {
                    should_notify = true;
                    ++it;
                    continue;
                }
                LON_ASSERT(it->cb || it->fiber);
                if (it->fiber && it->fiber->getState() == fiber::Fiber::EXEC)
                {
                    ++it;
                    continue;
                }
                task = *it;
                m_tasks.erase(it);
                ++m_active_threads_count;
                is_active = true;
                break;
            }
        }
        if (should_notify)
        {
            notify();
        }
        if (task.fiber && task.fiber->getState() != fiber::Fiber::TERM &&
            task.fiber->getState() != fiber::Fiber::ERROR)
        {
            task.fiber->swapIn(getMainFiber());
            --m_active_threads_count;
            if (task.fiber->getState() == fiber::Fiber::READY)
            {
                schedule(task.fiber);
            }
            else if (task.fiber->getState() != fiber::Fiber::TERM &&
                     task.fiber->getState() != fiber::Fiber::ERROR)
            {
                task.fiber->setState(fiber::Fiber::HOLD);
            }
            task.reset();
        }
        else if (task.cb)
        {
            if (cb_fiber)
            {
                cb_fiber->reset(task.cb);
            }
            else
            {
                cb_fiber.reset(new fiber::Fiber(task.cb, m_fiber_stack_size));
            }
            task.reset();
            cb_fiber->swapIn(getMainFiber());
            --m_active_threads_count;
            if (cb_fiber->getState() == fiber::Fiber::READY)
            {
                schedule(cb_fiber);
                cb_fiber.reset();
            }
            else if (cb_fiber->getState() == fiber::Fiber::TERM ||
                     cb_fiber->getState() == fiber::Fiber::ERROR)
            {
                cb_fiber->reset(nullptr);
            }
            // else if (cb_fiber->getState() != fiber::Fiber::TERM)
            else
            {
                cb_fiber->setState(fiber::Fiber::HOLD);
                cb_fiber.reset();
            }
        }
        else
        {
            if (is_active)
            {
                --m_active_threads_count;
                continue;
            }
            if (idle_fiber->getState() == fiber::Fiber::TERM)
            {
                LON_DEBUG(LON_LOG_ROOT) << "idle fiber terminate";
                break;
            }
            ++m_idle_threads_count;
            idle_fiber->swapIn(getMainFiber());
            --m_idle_threads_count;
            if (idle_fiber->getState() != fiber::Fiber::TERM &&
                idle_fiber->getState() != fiber::Fiber::ERROR)
            {
                idle_fiber->setState(fiber::Fiber::HOLD);
            }
        }
    }
}

bool Scheduler::stopping()
{
    MutexType::Lock lock(m_mutex);
    return m_stopping && m_auto_stop && m_tasks.empty() && m_active_threads_count == 0;
}

void Scheduler::idle()
{
    LON_DEBUG(LON_LOG_ROOT) << "idle";
    while (!stopping())
    {
        fiber::Fiber::yieldToHold(getMainFiber());
    }
}

void Scheduler::setThis() { t_cur_scheduler = this; }

bool Scheduler::hasIdleThreads() { return m_idle_threads_count > 0; }

size_t Scheduler::getTaskCount() const
{
    MutexType::Lock lock(m_mutex);
    return m_tasks.size();
}

//静态函数
Scheduler *Scheduler::getThis() { return t_cur_scheduler; }

fiber::Fiber *Scheduler::getMainFiber() { return t_scheduler_fiber; }

scheduler::Scheduler::Task::Task() : cb(nullptr), fiber(nullptr), thread_id(-1) {}

scheduler::Scheduler::Task::Task(fiber::Fiber::Ptr f, int t) : cb(nullptr), fiber(f), thread_id(t)
{
}

scheduler::Scheduler::Task::Task(fiber::Fiber::Ptr *f, int t) : cb(nullptr), thread_id(t)
{
    fiber.swap(*f);
}

scheduler::Scheduler::Task::Task(std::function<void()> f, int t)
    : cb(f), fiber(nullptr), thread_id(t)
{
}

scheduler::Scheduler::Task::Task(std::function<void()> *f, int t) : fiber(nullptr), thread_id(t)
{
    cb.swap(*f);
}

void scheduler::Scheduler::Task::reset()
{
    cb        = nullptr;
    fiber     = nullptr;
    thread_id = -1;
}

} // namespace scheduler
} // namespace lon