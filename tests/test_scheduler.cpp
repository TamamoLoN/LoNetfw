#include "lonetfw/lonetfw.h"

static auto g_logger = LON_LOG_ROOT;

void test_fiber()
{
    static int i = 5;
    LON_DEBUG(g_logger) << "test_fiber cnt = " << i;
    // usleep(100000);
    if (--i == 0)
    {
        return;
    }
    lon::scheduler::Scheduler::getThis()->schedule(test_fiber);
}
void test_fiber1()
{
    static int i = 5;
    LON_DEBUG(g_logger) << "test_fiber cnt = " << i;
    // usleep(100000);
    if (--i == 0)
    {
        return;
    }
    lon::scheduler::Scheduler::getThis()->schedule(test_fiber1, lon::util::getThreadId());
}

void test_scheduler()
{
    auto worker = std::make_shared<lon::scheduler::Scheduler>(
        3, false, "main_worker", lon::config::GlobalConfig::Instance().config_fiber->getData());

    worker->start();
    worker->schedule(test_fiber);
    worker->stop();
}

void test_scheduler_caller()
{
    auto worker = std::make_shared<lon::scheduler::Scheduler>(
        3, true, "main_worker", lon::config::GlobalConfig::Instance().config_fiber->getData());

    worker->start();
    worker->schedule(test_fiber);
    worker->stop();
}

void test_scheduler_set_thread()
{
    auto worker = std::make_shared<lon::scheduler::Scheduler>(
        3, false, "main_worker", lon::config::GlobalConfig::Instance().config_fiber->getData());

    worker->start();
    worker->schedule(test_fiber1);
    worker->stop();
}

void test_scheduler_fiber()
{
    auto fiber = std::make_shared<lon::fiber::Fiber>(
        test_fiber, lon::config::GlobalConfig::Instance().config_fiber->getData());
    auto worker = std::make_shared<lon::scheduler::Scheduler>(
        3, false, "main_worker", lon::config::GlobalConfig::Instance().config_fiber->getData());

    worker->start();
    worker->schedule(fiber);
    worker->stop();
}

void test_scheduler_yield()
{
    auto worker = std::make_shared<lon::scheduler::Scheduler>(
        1, false, "main_worker", lon::config::GlobalConfig::Instance().config_fiber->getData());

    worker->start();
    worker->schedule([]() {
        LON_DEBUG(g_logger) << "test yield";
        auto fiber = lon::fiber::Fiber::getThis();
        auto w     = lon::scheduler::Scheduler::getThis();
        w->schedule(fiber);
        lon::fiber::Fiber::yieldToHold(w->getMainFiber());
        LON_DEBUG(g_logger) << "test yield end";
    });
    worker->stop();
}

int main(int argc, char const *argv[])
{
    lon::config::Config::parseFromYaml("./.config/log.yaml");
    // test_scheduler();
    // test_scheduler_caller();
    // test_scheduler_set_thread();
    test_scheduler_fiber();
    test_scheduler_yield();
    return 0;
}
