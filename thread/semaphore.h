#pragma once

#include "log/logger.h"
#include "util/noncopyable.h"
#include <errno.h>
#include <semaphore.h>

namespace lon
{
namespace thread
{
class Semaphore : public util::Nonecopyable
{
  public:
    explicit Semaphore(uint32_t count = 0);
    virtual ~Semaphore();

    void wait();
    void notify();

  private:
    sem_t m_semaphore;
    uint32_t m_count;
};
} // namespace thread
} // namespace lon