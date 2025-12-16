#include "lonetfw/lonetfw.h"

void test_fiber()
{
    const auto &io = lon::scheduler::IOScheduler::getThis();
    int fd         = socket(AF_INET, SOCK_STREAM, 0);
    LON_INFO(LON_LOG_ROOT) << "socket fd = " << fd;
    LON_ASSERT(fd != -1);
#ifdef _WIN32
    u_long nb = 1;
    ioctlsocket(fd, FIONBIO, &nb);
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);

    io->addEvent(fd, lon::scheduler::IOScheduler::Event::READ,
                 []() { LON_FATAL(LON_LOG_ROOT) << "read cb"; });
    io->addEvent(fd, lon::scheduler::IOScheduler::Event::WRITE,
                 [fd]()
                 {
                     auto fd_ = fd;
                     LON_FATAL(LON_LOG_ROOT) << "write cb :connected";
                     // lon::scheduler::IOScheduler::getThis()->cancelEvent(
                     //     fd_, lon::scheduler::IOScheduler::Event::READ);
                     // lon::scheduler::IOScheduler::getThis()->cancelAll(fd_);
                     closesocket(fd_);
                     LON_FATAL(LON_LOG_ROOT) << "close fd";
                 });
#else
    fcntl(fd, F_SETFL, O_NONBLOCK);
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);

    io->addEvent(fd, lon::scheduler::IOScheduler::Event::READ,
                 []() { LON_FATAL(LON_LOG_ROOT) << "read cb"; });
    io->addEvent(fd, lon::scheduler::IOScheduler::Event::WRITE,
                 [fd]()
                 {
                     auto fd_ = fd;
                     LON_FATAL(LON_LOG_ROOT) << "write cb :connected";
                     // lon::scheduler::IOScheduler::getThis()->cancelEvent(
                     //     fd_, lon::scheduler::IOScheduler::Event::READ);
                     lon::scheduler::IOScheduler::getThis()->cancelAll(fd_);
                     close(fd_);
                 });
#endif
    auto ret = connect(fd, (sockaddr *)&addr, sizeof(addr));
    if (ret == 0)
    {
        LON_INFO(LON_LOG_ROOT) << "connect immediately success";
        // 可以直接开始通信
    }
    else if (ret == -1 && errno == EINPROGRESS)
    {
        LON_INFO(LON_LOG_ROOT) << "connect in progress";
        // 等待可写事件，事件触发后再检查连接结果
    }
    else
    {
        LON_FATAL(LON_LOG_ROOT) << "connect failed: " << strerror(errno);
    }
}
auto fiber = std::make_shared<lon::fiber::Fiber>(
    test_fiber, lon::config::GlobalConfig::Instance().config_fiber->getData());
void test_io_scheduler()
{
    auto io = std::make_shared<lon::scheduler::IOScheduler>(
        2, false, "io_scheduler", lon::config::GlobalConfig::Instance().config_fiber->getData());
    // io->schedule(fiber);
    io->schedule(test_fiber);
}
lon::scheduler::Timer::Ptr timer = nullptr;
void test_timer()
{
    auto io = std::make_shared<lon::scheduler::IOScheduler>(
        2, true, "io_scheduler", lon::config::GlobalConfig::Instance().config_fiber->getData());
    timer = io->addTimer(
        50,
        []()
        {
            static int cnt = 0;
            LON_INFO(LON_LOG_ROOT)
                << "i am timer cnt=" << lon::util::lexical_cast<std::string>(cnt);

            if (++cnt > 10)
            {
                // timer->cancel();
                timer->reset(200, true);
            }
            if (cnt > 40)
            {
                timer->cancel();
            }
        },
        true);
    io->schedule(test_fiber);
}

int main(int argc, char const *argv[])
{
    lon::config::Config::parseFromYaml(".config/log.yaml");

    // test_io_scheduler();
    test_timer();
    return 0;
}
