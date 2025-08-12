#include "config/config.h"
#include "hook/hook.h"
#include <chrono>
#include <thread>

void test_sleep()
{
    auto io = std::make_shared<lon::scheduler::IOScheduler>(
        1, true, "io_scheduler", lon::config::ConfigInitter::Instance().config_fiber->getData());
    io->schedule([]() {
        sleep(1);
        LON_DEBUG(LON_LOG_ROOT) << "test sleep 10s";
    });
    io->schedule([&]() {
        io->schedule([]() {
            sleep(5);
            LON_DEBUG(LON_LOG_ROOT) << "test sleep 5s";
        });
        sleep(2);
        LON_DEBUG(LON_LOG_ROOT) << "test sleep 2s";
    });
    io->schedule([]() {
        sleep(3);
        LON_DEBUG(LON_LOG_ROOT) << "test sleep 3s";
    });

    sleep(1);
    LON_DEBUG(LON_LOG_ROOT) << "test sleep";
}
// lon::scheduler::IOScheduler::Ptr io = nullptr;
void test_ioscheduler_reschedule()
{
    auto io = std::make_shared<lon::scheduler::IOScheduler>(
        2, true, "io_scheduler", lon::config::ConfigInitter::Instance().config_fiber->getData());
    io->schedule([]() {
        LON_DEBUG(LON_LOG_ROOT) << "test sleep 1s";
        auto fiber = lon::fiber::Fiber::getThis();
        LON_DEBUG(LON_LOG_ROOT) << "fiberid=" << fiber->getFiberId();
        lon::scheduler::IOScheduler::getThis()->schedule(fiber);
        lon::fiber::Fiber::yieldToHold(lon::scheduler::IOScheduler::getMainFiber());
        // lon::fiber::Fiber::yieldToHold(lon::scheduler::IOScheduler::getMainFiber());
        LON_DEBUG(LON_LOG_ROOT) << "test sleep 2s";
    });
    // io->schedule([]() {
    //     std::this_thread::sleep_for(std::chrono::seconds(2));

    //     LON_DEBUG(LON_LOG_ROOT) << "test sleep 10s";
    // });
}

int main(int argc, char const *argv[])
{
    lon::config::Config::parseFromYaml(".config/log.yaml");

    // test_sleep();
    test_ioscheduler_reschedule();
    // sleep(1);
    // usleep(10000);

    return 0;
}
