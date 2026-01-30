#pragma once

#include "scheduler/ioscheduler.h"

#define SCHEDMGR lon::util::Singleton<lon::scheduler::SchedulerManager>::Instance()

namespace lon
{
namespace scheduler
{
class SchedulerManager
{
  public:
    explicit SchedulerManager();
    virtual ~SchedulerManager();

    void addScheduler(Scheduler::Ptr scheduler);
    Scheduler::Ptr getScheduler(const std::string &name);

    template <typename FiberOrCb>
    void schedule(const std::string &name, FiberOrCb fc, int thread = -1)
    {
        auto scheduler = getScheduler(name);
        if (scheduler)
        {
            scheduler->schedule(fc, thread);
        }
        else
        {
            LON_ERROR(LON_LOG_ROOT) << "scheduler name=" << name << " not exists";
        }
    }

    template <typename Iterator>
    void schedule(const std::string &name, Iterator begin, Iterator end)
    {
        auto scheduler = getScheduler(name);
        if (scheduler)
        {
            scheduler->schedule(begin, end);
        }
        else
        {
            LON_ERROR(LON_LOG_ROOT) << "scheduler name=" << name << " not exists";
        }
    }

    bool start();
    void stop();
    bool isStoped() const;
    uint32_t getSchedulerCount();

  private:
    std::unordered_map<std::string, std::vector<Scheduler::Ptr>> m_schedulers;
    bool m_stopping;
};
} // namespace scheduler
} // namespace lon