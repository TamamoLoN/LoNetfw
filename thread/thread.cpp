#include "thread/thread.h"

namespace lon
{
namespace thread
{
static thread_local Thread *t_thread          = nullptr;
static thread_local std::string t_thread_name = "UNKNOWN";

Thread::Thread(std::function<void()> cb, const std::string &name) : m_cb(cb), m_name(name), m_id(-1)
{
    int rt = pthread_create(&m_thread, nullptr, &Thread::run, this);
    if (rt)
    {
        std::stringstream ss;
        ss << "pthread create failed, rt = " << rt << ", name=" << m_name;
        LON_ERROR(LON_LOG_NAME("system")) << ss.str();
        throw std::runtime_error(ss.str());
    }
}
Thread::~Thread()
{
    if (m_thread)
    {
        pthread_detach(m_thread);
    }
}

pid_t Thread::getId() const { return m_id; }

void Thread::setId(const pid_t &id) { m_id = id; }

std::string Thread::getName() const { return m_name; }

void Thread::setName(const std::string &name) { m_name = name; }

void Thread::join()
{
    if (m_thread)
    {
        int rt = pthread_join(m_thread, nullptr);
        if (rt)
        {
            std::stringstream ss;
            ss << "pthread join failed, rt = " << rt << ", name=" << m_name;
            LON_ERROR(LON_LOG_NAME("system")) << ss.str();
            throw std::runtime_error(ss.str());
        }
        m_thread = 0;
    }
}

//静态成员函数
Thread *Thread::getThis() { return t_thread; }
const std::string &Thread::getNameStatic() { return t_thread_name; }
void Thread::setNameStatic(const std::string &name)
{
    if (t_thread != nullptr)
    {
        t_thread->setName(name);
    }
    t_thread_name = name;
}
void *Thread::run(void *arg)
{
    Thread *thread = (Thread *)arg;
    t_thread       = thread;
    thread->setId(util::getThreadId());
    pthread_setname_np(pthread_self(), thread->getName().substr(0, 15).c_str());
    std::function<void()> cb;
    // swap不会改变智能指针的引用次数
    cb.swap(thread->m_cb);
    cb();
    return nullptr;
}

} // namespace thread
} // namespace lon