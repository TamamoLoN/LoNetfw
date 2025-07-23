#include "scheduler/scheduler.h"

int main(int argc, char const *argv[])
{
    auto worker = std::make_shared<lon::scheduler::Scheduler>(1, true, "main_worker");
    worker->start();
    worker->stop();
    return 0;
}
