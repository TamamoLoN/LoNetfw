#include "config/config.h"
#include "thread/thread.h"

void test_thread()
{
    lon::thread::Thread::Ptr t =
        std::make_shared<lon::thread::Thread>([]() { std::cout << "hello" << std::endl; }, "test_");
    t->join();
}

int main(int argc, char const *argv[])
{
    lon::config::Config::parseFromYaml("./.config/log.yaml");
    test_thread();
    return 0;
}
