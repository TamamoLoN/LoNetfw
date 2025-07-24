#include "config/config.h"
#include "scheduler/scheduler.h"

void test_fiber()
{
    static int i = 5;
    LON_DEBUG(LON_LOG_ROOT) << "test_fiber cnt = " << i;
    usleep(100000);
    if (--i == 0)
    {
        return;
    }
    lon::scheduler::Scheduler::getThis()->schedule(test_fiber);
}
void test_fiber1()
{
    static int i = 5;
    LON_DEBUG(LON_LOG_ROOT) << "test_fiber cnt = " << i;
    usleep(100000);
    if (--i == 0)
    {
        return;
    }
    lon::scheduler::Scheduler::getThis()->schedule(test_fiber1, lon::util::getThreadId());
}

void test_scheduler()
{
    auto worker = std::make_shared<lon::scheduler::Scheduler>(
        3, false, "main_worker", lon::config::ConfigInitter::Instance().config_fiber->getData());

    worker->start();
    worker->schedule(test_fiber);
    worker->stop();
}

void test_scheduler_caller()
{
    auto worker = std::make_shared<lon::scheduler::Scheduler>(
        3, true, "main_worker", lon::config::ConfigInitter::Instance().config_fiber->getData());

    worker->start();
    worker->schedule(test_fiber);
    worker->stop();
}

void test_scheduler_set_thread()
{
    auto worker = std::make_shared<lon::scheduler::Scheduler>(
        3, false, "main_worker", lon::config::ConfigInitter::Instance().config_fiber->getData());

    worker->start();
    worker->schedule(test_fiber1);
    worker->stop();
}

int main(int argc, char const *argv[])
{
    lon::config::Config::parseFromYaml("./.config/log.yaml");
    // test_scheduler();
    test_scheduler_caller();
    // test_scheduler_set_thread();
    return 0;
}
