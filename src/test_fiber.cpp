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

void test_thread_fiber_()
{
    lon::thread::Thread::Ptr t1;
    lon::thread::Thread::Ptr t2;
    lon::thread::Thread::Ptr t3;

    t1 = std::make_shared<lon::thread::Thread>([]() { test_fiber(); }, "thread_1");
    t2 = std::make_shared<lon::thread::Thread>([]() { test_fiber(); }, "thread_2");
    t3 = std::make_shared<lon::thread::Thread>([]() { test_fiber(); }, "thread_3");

    t3->join();
    t1->join();
    t2->join();
}

int main(int argc, char const *argv[])
{
    lon::config::Config::parseFromYaml("./.config/log.yaml");
    test_fiber();
    /* FIXME - 这里用valgrind运行会出现报错，但是内存无泄漏
    Conditional jump or move depends on uninitialised value(s)
    ==15243== Use of uninitialised value of size 8*/
    // test_thread_fiber();
    // test_thread_fiber_();
    return 0;
}
