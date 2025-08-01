#include "config/config.h"
#include "scheduler/ioscheduler.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void test_fiber()
{
    const auto &io = lon::scheduler::IOScheduler::getThis();
    int fd         = socket(AF_INET, SOCK_STREAM, 0);
    LON_INFO(LON_LOG_ROOT) << "socket fd = " << fd;
    LON_ASSERT(fd != -1);
    fcntl(fd, F_SETFL, O_NONBLOCK);
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(80);
    inet_pton(AF_INET, "182.61.201.211", &addr.sin_addr.s_addr);

    io->addEvent(fd, lon::scheduler::IOScheduler::Event::READ,
                 []() { LON_FATAL(LON_LOG_ROOT) << "read cb"; });
    io->addEvent(fd, lon::scheduler::IOScheduler::Event::WRITE, [&fd]() {
        LON_FATAL(LON_LOG_ROOT) << "write cb :connected";
        lon::scheduler::IOScheduler::getThis()->cancelEvent(
            fd, lon::scheduler::IOScheduler::Event::READ);
        close(fd);
    });
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

void test1()
{
    auto io = std::make_shared<lon::scheduler::IOScheduler>(
        2, false, "io_scheduler", lon::config::ConfigInitter::Instance().config_fiber->getData());
    io->schedule(test_fiber);
}

int main(int argc, char const *argv[])
{
    lon::config::Config::parseFromYaml(".config/log.yaml");

    test1();
    return 0;
}
