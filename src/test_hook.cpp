#include "config/config.h"
#include "hook/hook.h"

void test_sleep()
{
    auto io = std::make_shared<lon::scheduler::IOScheduler>(
        1, false, "io_scheduler",
        lon::config::ConfigInitter::Instance().config_fiber->getData());
    io->schedule([]() {
        sleep(10);
        LON_DEBUG(LON_LOG_ROOT) << "test sleep 10s";
    });
    io->schedule([]() {
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

int main(int argc, char const *argv[])
{
    lon::config::Config::parseFromYaml(".config/log.yaml");

    test_sleep();
    // sleep(1);
    // usleep(10000);

    return 0;
}
