#include "scheduler/scheduler.h"

namespace lon
{
namespace scheduler
{
static thread_local Scheduler *t_scheduler = nullptr;
static thread_local fiber::Fiber *t_fiber  = nullptr;

Scheduler::Scheduler(size_t threads_count, bool use_caller, std::string name)
    : m_threads_count(threads_count), m_active_threads_count(0), m_idle_threads_count(0),
      m_root_thread_id(-1), m_stopping(true), m_auto_stop(false), m_use_caller(use_caller),
      m_name(name)
{
    LON_ASSERT(m_threads_count > 0);
    if (m_use_caller)
    {
        fiber::Fiber::getThis();
        --m_threads_count;
        LON_ASSERT(getThis() == nullptr);
        t_scheduler = this;
        m_root_fiber.reset(
            new fiber::Fiber(std::bind(&Scheduler::run, this),
                             config::ConfigInitter::Instance().config_fiber->getData()));
        thread::Thread::setNameStatic(m_name);
        t_fiber          = m_root_fiber.get();
        m_root_thread_id = thread::Thread::getIdStatic();
        m_threads_id.push_back(m_root_thread_id);
    }
}

Scheduler::~Scheduler()
{
    LON_ASSERT_(m_stopping, "schedule never stop!");
    if (getThis() == this)
    {
        t_scheduler = nullptr;
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
    for (int cnt = 0; cnt < m_threads_count; cnt++)
    {
        m_threads.push_back(std::make_shared<thread::Thread>(
            std::bind(&Scheduler::run, this), m_name + "_" + util::lexical_cast<std::string>(cnt)));
        m_threads_id.push_back(m_threads[cnt]->getId());
    }
    lock.unlock();
    if (m_root_fiber)
    {
        m_root_fiber->swapIn();
    }
    LON_INFO(LON_LOG_ROOT) << "scheduler[" << m_name << "]:" << this << " started";
}

void Scheduler::stop()
{
    // MutexType::Lock lock(m_mutex);
    m_auto_stop = true;
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
    if (m_root_thread_id != -1)
    {
        LON_ASSERT(this == getThis());
    }
    else
    {
        LON_ASSERT(this != getThis());
    }
    m_stopping = true;
    for (size_t cnt = 0; cnt < m_threads_count; cnt++)
    {
        notify();
    }
    if (m_root_fiber)
    {
        notify();
    }
    if (stopping())
    {
        return;
    }
}

void Scheduler::notify() { LON_DEBUG(LON_LOG_ROOT) << "notify"; }

void Scheduler::run()
{
    setThis();
    if (thread::Thread::getIdStatic() != m_root_thread_id)
    {
        t_fiber = fiber::Fiber::getThis().get();
    }
    fiber::Fiber::Ptr idle_fiber(
        new fiber::Fiber(std::bind(&Scheduler::idle, this),
                         config::ConfigInitter::Instance().config_fiber->getData()));

    fiber::Fiber::Ptr cb_fiber;
    TFWrapper tf;

    while (true)
    {
        tf.reset();
        bool should_notify = false;
        {
            MutexType::Lock lock(m_mutex);
            auto it = m_fibers.begin();
            while (it != m_fibers.end())
            {
                if (it->thread_id != -1 && it->thread_id != thread::Thread::getIdStatic())
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
                tf = *it;
                m_fibers.erase(it);
            }
        }
        if (should_notify)
        {
            notify();
        }
        if (tf.fiber && tf.fiber->getState() != fiber::Fiber::TERM &&
            tf.fiber->getState() != fiber::Fiber::ERROR)
        {
            ++m_active_threads_count;
            tf.fiber->swapIn(getMainFiber());
            --m_active_threads_count;
            if (tf.fiber->getState() == fiber::Fiber::READY)
            {
                schedule(tf.fiber);
            }
            else if (tf.fiber->getState() != fiber::Fiber::TERM &&
                     tf.fiber->getState() != fiber::Fiber::ERROR)
            {
                tf.fiber->setState(fiber::Fiber::HOLD);
            }
            tf.reset();
        }
        else if (tf.cb)
        {
            if (cb_fiber)
            {
                cb_fiber->reset(tf.cb);
            }
            else
            {
                cb_fiber.reset(new fiber::Fiber(
                    tf.cb, config::ConfigInitter::Instance().config_fiber->getData()));
            }
            tf.reset();
            ++m_active_threads_count;
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
                cb_fiber.reset();
            }
            // else if (cb_fiber->getState() != fiber::Fiber::TERM)
            else
            {
                tf.fiber->setState(fiber::Fiber::HOLD);
                cb_fiber.reset();
            }
        }
        else
        {
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
    return m_stopping && m_auto_stop && m_fibers.empty() && m_active_threads_count == 0;
}

void Scheduler::idle() { LON_DEBUG(LON_LOG_ROOT) << "idle"; }

void Scheduler::setThis() { t_scheduler = this; }

//静态函数
Scheduler *Scheduler::getThis() { return t_scheduler; }

fiber::Fiber *Scheduler::getMainFiber() { return t_fiber; }

scheduler::Scheduler::TFWrapper::TFWrapper() : cb(nullptr), fiber(nullptr), thread_id(-1) {}

scheduler::Scheduler::TFWrapper::TFWrapper(fiber::Fiber::Ptr f, int t)
    : cb(nullptr), fiber(f), thread_id(t)
{
}

scheduler::Scheduler::TFWrapper::TFWrapper(fiber::Fiber::Ptr *f, int t) : cb(nullptr), thread_id(t)
{
    fiber.swap(*f);
}

scheduler::Scheduler::TFWrapper::TFWrapper(std::function<void()> f, int t)
    : cb(f), fiber(nullptr), thread_id(t)
{
}

scheduler::Scheduler::TFWrapper::TFWrapper(std::function<void()> *f, int t)
    : fiber(nullptr), thread_id(t)
{
    cb.swap(*f);
}

void scheduler::Scheduler::TFWrapper::reset()
{
    cb        = nullptr;
    fiber     = nullptr;
    thread_id = -1;
}

} // namespace scheduler
} // namespace lon