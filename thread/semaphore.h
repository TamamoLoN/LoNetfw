#pragma once

#include "util/noncopyable.h"
#include "util/macro.h"
#include <errno.h>
#include <semaphore.h>
#include <sstream>
#include <stdint.h>

namespace lon
{
namespace thread
{
class LON_API Semaphore : public util::Nonecopyable
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