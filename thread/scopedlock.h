#pragma once
#include "util/noncopyable.h"
namespace lon
{
namespace thread
{
//以下mutex均为引用传递，防止赋值构造
template <typename T> class ScopedLock : public util::Nonecopyable
{
  public:
    explicit ScopedLock(T &mutex) : m_mutex(mutex), m_locked(false) { lock(); }

    virtual ~ScopedLock() { unlock(); }

    void lock()
    {
        if (!m_locked)
        {
            m_mutex.lock();
            m_locked = true;
        }
    }

    void unlock()
    {
        if (m_locked)
        {
            m_mutex.unlock();
            m_locked = false;
        }
    }

  private:
    T &m_mutex;
    bool m_locked;
};

template <typename T> class ScopedRdLock : public util::Nonecopyable
{
  public:
    explicit ScopedRdLock(T &mutex) : m_mutex(mutex), m_locked(false) { lock(); }

    virtual ~ScopedRdLock() { unlock(); }

    void lock()
    {
        if (!m_locked)
        {
            m_mutex.rdlock();
            m_locked = true;
        }
    }

    void unlock()
    {
        if (m_locked)
        {
            m_mutex.unlock();
            m_locked = false;
        }
    }

  private:
    T &m_mutex;
    bool m_locked;
};

template <typename T> class ScopedWrLock : public util::Nonecopyable
{
  public:
    explicit ScopedWrLock(T &mutex) : m_mutex(mutex), m_locked(false) { lock(); }

    virtual ~ScopedWrLock() { unlock(); }

    void lock()
    {
        if (!m_locked)
        {
            m_mutex.wrlock();
            m_locked = true;
        }
    }

    void unlock()
    {
        if (m_locked)
        {
            m_mutex.unlock();
            m_locked = false;
        }
    }

  private:
    T &m_mutex;
    bool m_locked;
};

} // namespace thread
} // namespace lon