#include "config/config.h"
#include "fiber/fiber.h"
#include "thread/thread.h"

void test_fiber()
{
    try
    {
        {
            //创建主协程
            auto main_fiber = lon::fiber::Fiber::getThis();
            LON_INFO(LON_LOG_ROOT) << "main begin";
            auto fiber = std::make_shared<lon::fiber::Fiber>(
                []() {
                    LON_INFO(LON_LOG_ROOT) << "run in fiber begin";
                    lon::fiber::Fiber::yieldToHold();
                    LON_INFO(LON_LOG_ROOT) << "run in fiber end";
                    lon::fiber::Fiber::yieldToHold();
                },
                lon::config::ConfigInitter::Instance().config_fiber->getData());
            fiber->swapIn();
            LON_INFO(LON_LOG_ROOT) << "main after swapIn";
            fiber->swapIn();
            LON_INFO(LON_LOG_ROOT) << "main after end";
            fiber->swapIn();
        }
        LON_INFO(LON_LOG_ROOT) << "main end";
    }
    catch (const std::runtime_error &e)
    {
        LON_INFO(LON_LOG_ROOT) << e.what();
    }
    // catch (...)
    // {
    //     LON_ASSERT(false)
    // }
}

void test_thread_fiber()
{
    std::vector<lon::thread::Thread::Ptr> threads;
    for (int i = 0; i < 3; i++)
    {

        threads.push_back(std::make_shared<lon::thread::Thread>([]() { test_fiber(); },
                                                                "thread_" + std::to_string(i)));
    }
    for (auto it : threads)
    {
        it->join();
    }
}

int main(int argc, char const *argv[])
{
    lon::config::Config::parseFromYaml("./.config/log.yaml");
    // test_fiber();
    test_thread_fiber();
    return 0;
}
