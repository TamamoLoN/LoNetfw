#include "thread/semaphore.h"

namespace lon
{
namespace thread
{
Semaphore::Semaphore(uint32_t count) : m_count(count)
{
    int rt = sem_init(&m_semaphore, 0, count);
    if (rt != 0)
    {
        std::stringstream ss;
        ss << "sem_init failed, rt = " << rt;
        throw std::runtime_error(ss.str());
    }
}

Semaphore::~Semaphore() { sem_destroy(&m_semaphore); }

void Semaphore::wait()
{
    while (true)
    {
        if (sem_wait(&m_semaphore) == 0)
        {
            break;
        }
    }
}

void Semaphore::notify()
{
    int rt = sem_post(&m_semaphore);
    if (rt != 0)
    {
        std::stringstream ss;
        ss << "sem_post failed, rt = " << rt;
        throw std::runtime_error(ss.str());
    }
}

} // namespace thread
} // namespace lon