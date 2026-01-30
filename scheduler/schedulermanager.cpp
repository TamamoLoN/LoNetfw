#include "scheduler/schedulermanager.h"

namespace lon
{
namespace scheduler
{
SchedulerManager::SchedulerManager() : m_stopping(true), m_schedulers({}) {}

SchedulerManager::~SchedulerManager() { stop(); }

void SchedulerManager::addScheduler(Scheduler::Ptr scheduler)
{
    m_schedulers[scheduler->getName()].emplace_back(std::move(scheduler));
}

Scheduler::Ptr SchedulerManager::getScheduler(const std::string &name)
{
    auto it = m_schedulers.find(name);
    if (it == m_schedulers.end())
    {
        return nullptr;
    }
    if (it->second.size() == 1)
    {
        return it->second[0];
    }
    return it->second[rand() % it->second.size()];
}

bool SchedulerManager::start()
{
    if (!m_stopping)
    {
        return false;
    }
    for (const auto &schedulers : m_schedulers)
    {
        for (const auto &scheduler : schedulers.second)
        {
            scheduler->start();
        }
    }
    m_stopping = m_schedulers.empty();
    return !m_stopping;
}

void SchedulerManager::stop()
{
    if (m_stopping)
    {
        return;
    }
    for (const auto &schedulers : m_schedulers)
    {
        for (const auto &scheduler : schedulers.second)
        {
            scheduler->schedule([]() {});
            scheduler->stop();
        }
    }
    m_schedulers.clear();
    m_stopping = true;
}

bool SchedulerManager::isStoped() const { return m_stopping; }

uint32_t SchedulerManager::getSchedulerCount()
{
    int32_t count = 0;
    for (const auto &schedulers : m_schedulers)
    {
        count += schedulers.second.size();
    }
    return count;
}

} // namespace scheduler
} // namespace lon