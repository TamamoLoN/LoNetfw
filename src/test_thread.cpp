#include "config/config.h"
#include "thread/thread.h"

void test_thread_no_join()
{
    auto t = std::make_shared<lon::thread::Thread>(
        []() { LON_INFO(LON_LOG_ROOT) << "id=" << lon::util::getThreadId(); }, "test");
    //析构里detach，但是主线程比线程早结束了，所以崩溃
    t->join();
}

void test_thread()
{
    std::vector<lon::thread::Thread::Ptr> threads;
    for (int i = 0; i < 10; i++)
    {
        threads.push_back(std::make_shared<lon::thread::Thread>(
            []() {
                LON_INFO(LON_LOG_ROOT)
                    << "id=" << lon::util::getThreadId()
                    << "; this->id=" << lon::thread::Thread::getThis()->getId()
                    << "; name=" << lon::thread::Thread::getNameStatic()
                    << "; this->name=" << lon::thread::Thread::getThis()->getName();
            },
            "thread_" + std::to_string(i)));
    }

    for (const auto &it : threads)
    {
        // it->join();
    }
}

int main(int argc, char const *argv[])
{
    lon::config::Config::parseFromYaml("./.config/log.yaml");

    // test_thread_no_join();
    test_thread();
    return 0;
}
