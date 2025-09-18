#include "lonetfw/lonetfw.h"
#include <chrono>

void test_sleep()
{
    auto durationWrapper = [](std::function<void()> cb) {
        auto start = std::chrono::steady_clock::now();
        cb();
        auto end      = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        LON_DEBUG(LON_LOG_ROOT) << "test sleep duration=" << duration;
    };
    auto io = std::make_shared<lon::scheduler::IOScheduler>(
        1, true, "io_scheduler", lon::config::GlobalConfig::Instance().config_fiber->getData());
    io->schedule([=]() {
        durationWrapper([]() {
            sleep(10);
            LON_DEBUG(LON_LOG_ROOT) << "test sleep 10s";
        });
    });
    io->schedule([&]() {
        io->schedule([=]() {
            durationWrapper([]() {
                usleep(5000000);
                LON_DEBUG(LON_LOG_ROOT) << "test sleep 5s";
            });
        });
        durationWrapper([]() {
            sleep(2);
            LON_DEBUG(LON_LOG_ROOT) << "test sleep 2s";
        });
    });
    io->schedule([=]() {
        durationWrapper([]() {
            struct timespec ts;
            ts.tv_sec  = 3;
            ts.tv_nsec = 0;
            nanosleep(&ts, nullptr);
            LON_DEBUG(LON_LOG_ROOT) << "test sleep 3s";
        });
    });

    sleep(1);
    LON_DEBUG(LON_LOG_ROOT) << "test sleep";
}
// lon::scheduler::IOScheduler::Ptr io = nullptr;
void test_ioscheduler_reschedule()
{
    auto io = std::make_shared<lon::scheduler::IOScheduler>(
        2, true, "io_scheduler", lon::config::GlobalConfig::Instance().config_fiber->getData());
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

void test_socket()
{
    auto io = std::make_shared<lon::scheduler::IOScheduler>(
        1, true, "io_scheduler", lon::config::GlobalConfig::Instance().config_fiber->getData());
    io->schedule([]() {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        LON_INFO(LON_LOG_ROOT) << "socket fd = " << fd;
        LON_ASSERT(fd != -1);
        // fcntl(fd, F_SETFL, O_NONBLOCK);
        sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port   = htons(80);
        inet_pton(AF_INET, "182.61.201.211", &addr.sin_addr.s_addr);

        int ret = connect(fd, (sockaddr *)&addr, sizeof(addr));
        LON_INFO(LON_LOG_ROOT) << "connect ret = " << ret << " ,errno=" << errno;
        if (ret)
        {
            LON_ERROR(LON_LOG_ROOT) << "connect error";
            return;
        }

        const char *data = "GET / HTTP/1.0\r\n\r\n";
        ret              = send(fd, data, strlen(data), 0);
        LON_INFO(LON_LOG_ROOT) << "send ret = " << ret << " ,errno=" << errno;
        if (ret <= 0)
        {
            LON_ERROR(LON_LOG_ROOT) << "send error";
            return;
        }

        std::string buf;
        buf.resize(4096);
        ret = recv(fd, &buf[0], buf.size(), 0);
        LON_INFO(LON_LOG_ROOT) << "recv ret = " << ret << " ,errno=" << errno;
        if (ret <= 0)
        {
            LON_ERROR(LON_LOG_ROOT) << "recv error";
            return;
        }
        buf.resize(ret);
        LON_INFO(LON_LOG_ROOT) << "recv = " << buf;
    });
}

int main(int argc, char const *argv[])
{
    lon::config::Config::parseFromYaml(".config/log.yaml");

    // test_sleep();
    // test_ioscheduler_reschedule();
    test_socket();
    // sleep(1);
    // usleep(10000);

    return 0;
}
