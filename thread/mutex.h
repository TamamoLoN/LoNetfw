#pragma once
#include "thread/scopedlock.h"
#include <pthread.h>
#include <sstream>
namespace lon
{
namespace thread
{
//普通锁
class Mutex : public util::Nonecopyable
{
  public:
    using Lock = ScopedLock<Mutex>;
    Mutex();
    virtual ~Mutex();
    void lock();
    void unlock();

  private:
    pthread_mutex_t m_lock;
};

//读写锁
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
    pthread_rwlock_t m_lock;
};

} // namespace thread
} // namespace lon