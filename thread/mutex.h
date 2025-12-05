#pragma once
#include "thread/scopedlock.h"
#include <atomic>
#include <sstream>
#ifdef _WIN32
#define _TIMESPEC_DEFINED
#include <pthread.h>
#else
#include <pthread.h>
#endif

namespace lon
{
namespace thread
{
// 普通锁
class MutexNull : public util::Nonecopyable
{
  public:
    using Lock           = ScopedLock<MutexNull>;
    MutexNull()          = default;
    virtual ~MutexNull() = default;
    void lock() {}
    void unlock() {}
};

class Mutex : public util::Nonecopyable
{
  public:
    using Lock = ScopedLock<Mutex>;
    Mutex();
    virtual ~Mutex();
    void lock();
    void unlock();

  private:
    mutable pthread_mutex_t m_lock;
};

// 读写锁
class RWMutexNull : public util::Nonecopyable
{
  public:
    using RdLock           = ScopedRdLock<RWMutexNull>;
    using WrLock           = ScopedWrLock<RWMutexNull>;
    RWMutexNull()          = default;
    virtual ~RWMutexNull() = default;
    void rdlock() {}
    void wrlock() {}
    void unlock() {}
};

class RWMutex : public util::Nonecopyable
{
  public:
    using RdLock = ScopedRdLock<RWMutex>;
    using WrLock = ScopedWrLock<RWMutex>;
    RWMutex();
    virtual ~RWMutex();
    void rdlock();
    void wrlock();
    void unlock();

  private:
    mutable pthread_rwlock_t m_lock;
};

class SpinLock : public util::Nonecopyable
{
  public:
    using Lock = ScopedLock<SpinLock>;
    SpinLock();
    virtual ~SpinLock();
    void lock();
    void unlock();

  private:
    mutable pthread_spinlock_t m_lock;
};

} // namespace thread
} // namespace lon