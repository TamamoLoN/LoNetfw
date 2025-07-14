#include "thread/mutex.h"

namespace lon
{
namespace thread
{
Mutex::Mutex()
{
    int rt = pthread_mutex_init(&m_lock, nullptr);
    if (rt != 0)
    {
        std::stringstream ss;
        ss << "pthread_mutex_init failed, rt = " << rt;
        throw std::runtime_error(ss.str());
    }
}

Mutex::~Mutex() { pthread_mutex_destroy(&m_lock); }

void Mutex::lock()
{
    int rt = pthread_mutex_lock(&m_lock);
    if (rt != 0)
    {
        std::stringstream ss;
        ss << "pthread_mutex_lock failed, rt = " << rt;
        throw std::runtime_error(ss.str());
    }
}

void Mutex::unlock()
{
    int rt = pthread_mutex_unlock(&m_lock);
    if (rt != 0)
    {
        std::stringstream ss;
        ss << "pthread_mutex_unlock failed, rt = " << rt;
        throw std::runtime_error(ss.str());
    }
}

RWMutex::RWMutex()
{
    int rt = pthread_rwlock_init(&m_lock, nullptr);
    if (rt != 0)
    {
        std::stringstream ss;
        ss << "pthread_rwlock_init failed, rt = " << rt;
        throw std::runtime_error(ss.str());
    }
}

RWMutex::~RWMutex() { pthread_rwlock_destroy(&m_lock); }

void RWMutex::rdlock()
{
    int rt = pthread_rwlock_rdlock(&m_lock);
    if (rt != 0)
    {
        std::stringstream ss;
        ss << "pthread_rwlock_rdlock failed, rt = " << rt;
        throw std::runtime_error(ss.str());
    }
}

void RWMutex::wrlock()
{
    int rt = pthread_rwlock_wrlock(&m_lock);
    if (rt != 0)
    {
        std::stringstream ss;
        ss << "pthread_rwlock_wrlock failed, rt = " << rt;
        throw std::runtime_error(ss.str());
    }
}

void RWMutex::unlock()
{
    int rt = pthread_rwlock_unlock(&m_lock);
    if (rt != 0)
    {
        std::stringstream ss;
        ss << "pthread_rwlock_unlock failed, rt = " << rt;
        throw std::runtime_error(ss.str());
    }
}

SpinLock::SpinLock()
{
    int rt = pthread_spin_init(&m_lock, 0);
    if (rt != 0)
    {
        std::stringstream ss;
        ss << "pthread_spin_init failed, rt = " << rt;
        throw std::runtime_error(ss.str());
    }
}

SpinLock::~SpinLock() { pthread_spin_destroy(&m_lock); }

void SpinLock::lock()
{
    int rt = pthread_spin_lock(&m_lock);
    if (rt != 0)
    {
        std::stringstream ss;
        ss << "pthread_spin_lock failed, rt = " << rt;
        throw std::runtime_error(ss.str());
    }
}

void SpinLock::unlock()
{
    int rt = pthread_spin_unlock(&m_lock);
    if (rt != 0)
    {
        std::stringstream ss;
        ss << "pthread_spin_lock failed, rt = " << rt;
        throw std::runtime_error(ss.str());
    }
}

} // namespace thread
} // namespace lon