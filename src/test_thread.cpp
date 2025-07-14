#include "config/config.h"
#include "thread/thread.h"
#include <chrono>

void test_thread_no_join()
{
    auto t = std::make_shared<lon::thread::Thread>(
        []() { LON_INFO(LON_LOG_ROOT) << "id=" << lon::util::getThreadId(); }, "test");

    //析构里detach，但是主线程比线程早结束了，所以崩溃
    t->join();
    lon::thread::Thread t1([]() {}, "t1");
    // auto t2 = t1;
    // auto t3(t1);
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
        it->join();
    }
}
int cnt = 0;
void test_thread_mutex()
{
    std::vector<lon::thread::Thread::Ptr> threads;
    lon::thread::Mutex mtx;
    lon::thread::RWMutex rwmtx;

    for (int i = 0; i < 5; i++)
    {
        threads.push_back(std::make_shared<lon::thread::Thread>(
            [&]() {
                for (int j = 0; j < 1000000; j++)
                {
                    // lon::thread::Mutex::Lock lock(mtx);
                    // lon::thread::RWMutex::RdLock lock(rwmtx);
                    lon::thread::RWMutex::WrLock lock(rwmtx);
                    // LON_INFO(LON_LOG_ROOT) << cnt;
                    cnt++;
                }
            },
            "thread_" + std::to_string(i)));
    }

    for (const auto &it : threads)
    {
        it->join();
    }
    LON_INFO(LON_LOG_ROOT) << cnt;
}

void test_thread_mutex_log()
{
    std::vector<lon::thread::Thread::Ptr> threads;
    // lon::thread::Mutex mtx;
    auto start = std::chrono::high_resolution_clock::now();

    int k = 0;
    for (int i = 0; i < 1; i++)
    {
        auto t1 = std::make_shared<lon::thread::Thread>(
            [&]() {
                int j = 0;
                while (1)
                {
                    // lon::thread::Mutex::Lock lock(mtx);
                    LON_INFO(LON_LOG_NAME("root")) << "*************************************";
                    cnt++;
                }
            },
            "thread_" + std::to_string(k++));
        // auto t2 = std::make_shared<lon::thread::Thread>(
        //     [&]() {
        //         int j = 0;
        //         while (1)
        //         {
        //             // lon::thread::Mutex::Lock lock(mtx);
        //             LON_INFO(LON_LOG_NAME("root")) << "-------------------------------------";
        //             cnt++;
        //         }
        //     },
        //     "thread_" + std::to_string(k++));
        threads.push_back(t1);
        // threads.push_back(t2);
    }

    for (const auto &it : threads)
    {
        it->join();
    }
    auto end = std::chrono::high_resolution_clock::now();

    LON_INFO(LON_LOG_ROOT)
        << "duration: "
        << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
        << "us; write count:" << cnt;
}

int main(int argc, char const *argv[])
{
    lon::config::Config::parseFromYaml("./.config/log.yaml");

    // test_thread_no_join();
    // test_thread();
    // test_thread_mutex();
    test_thread_mutex_log();

    return 0;
}
