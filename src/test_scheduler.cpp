#include "config/config.h"
#include "scheduler/scheduler.h"

int main(int argc, char const *argv[])
{
    lon::config::Config::parseFromYaml("./.config/log.yaml");
    auto worker = std::make_shared<lon::scheduler::Scheduler>(1, true, "main_worker");
    worker->start();
    worker->stop();
    return 0;
}
