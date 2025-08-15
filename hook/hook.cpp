#include "hook/hook.h"

namespace lon
{
namespace hook
{

#define HOOK_FUN(XX)                                                                               \
    XX(sleep)                                                                                      \
    XX(usleep)                                                                                     \
    XX(nanosleep)                                                                                  \
    XX(socket)                                                                                     \
    XX(connect)                                                                                    \
    XX(accept)                                                                                     \
    XX(bind)                                                                                       \
    XX(read)                                                                                       \
    XX(readv)                                                                                      \
    XX(recv)                                                                                       \
    XX(recvfrom)                                                                                   \
    XX(write)                                                                                      \
    XX(writev)                                                                                     \
    XX(send)                                                                                       \
    XX(sendto)                                                                                     \
    XX(sendmsg)                                                                                    \
    XX(close)                                                                                      \
    XX(fcntl)                                                                                      \
    XX(ioctl)                                                                                      \
    XX(getsockopt)                                                                                 \
    XX(setsockopt)

Hook::Hook()
{
    static int is_inited = false;
    if (is_inited)
    {
        return;
    }
#define XX(name) name##_f = (name##_fun)dlsym(RTLD_NEXT, #name);
    HOOK_FUN(XX)
#undef XX
}

Hook::~Hook() {}

Hook &Hook::Instance()
{
    static Hook instance;
    return instance;
}

void Hook::enable() { util::HookState::enable(); }

void Hook::disable() { util::HookState::disable(); }

auto s_hook = Hook::Instance();

} // namespace hook
} // namespace lon
extern "C"
{
#define XX(name) name##_fun name##_f = nullptr;
    HOOK_FUN(XX)
#undef XX

    unsigned int sleep(unsigned int seconds)
    {
        if (!lon::util::HookState::isEnable())
        {
            return sleep_f(seconds);
        }
        auto fiber = lon::fiber::Fiber::getThis();
        auto ios   = lon::scheduler::IOScheduler::getThis();
        ios->addTimer(
            seconds * 1000, [fiber, ios]() { ios->schedule(fiber); }, false);
        lon::fiber::Fiber::yieldToHold(lon::scheduler::IOScheduler::getMainFiber());

        return 0;
    }

    int usleep(useconds_t usec)
    {
        if (!lon::util::HookState::isEnable())
        {
            return usleep_f(usec);
        }
        auto fiber = lon::fiber::Fiber::getThis();
        auto ios   = lon::scheduler::IOScheduler::getThis();
        ios->addTimer(
            usec / 1000, [fiber, ios]() { ios->schedule(fiber); }, false);
        lon::fiber::Fiber::yieldToHold(lon::scheduler::IOScheduler::getMainFiber());

        return 0;
    }

    int nanosleep(const struct timespec *req, struct timespec *rem)
    {
        if (!lon::util::HookState::isEnable())
        {
            return nanosleep_f(req, rem);
        }
        auto msec  = req->tv_sec * 1000 + req->tv_nsec / (1000 * 1000);
        auto fiber = lon::fiber::Fiber::getThis();
        auto ios   = lon::scheduler::IOScheduler::getThis();
        ios->addTimer(
            msec, [fiber, ios]() { ios->schedule(fiber); }, false);
        lon::fiber::Fiber::yieldToHold(lon::scheduler::IOScheduler::getMainFiber());

        return 0;
    }

    int socket(int domain, int type, int protocol)
    {
        if (!lon::util::HookState::isEnable())
        {
            return socket_f(domain, type, protocol);
        }
        return 0;
    }

    // int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
    // {
    //     if (!lon::util::HookState::isEnable())
    //     {
    //         return connect_f(sockfd, addr, addrlen);
    //     }
    //     return 0;
    // }

    // int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
    // {
    //     if (!lon::util::HookState::isEnable())
    //     {
    //         return accept_f(sockfd, addr, addrlen);
    //     }
    //     return 0;
    // }

    // int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
    // {
    //     if (!lon::util::HookState::isEnable())
    //     {
    //         return bind_f(sockfd, addr, addrlen);
    //     }
    //     return 0;
    // }

    // ssize_t read(int fd, void *buf, size_t count)
    // {
    //     if (!lon::util::HookState::isEnable())
    //     {
    //         return read_f(fd, buf, count);
    //     }
    //     return 0;
    // }

    // ssize_t readv(int fd, const struct iovec *iov, int iovcnt)
    // {
    //     if (!lon::util::HookState::isEnable())
    //     {
    //         return readv_f(fd, iov, iovcnt);
    //     }
    //     return 0;
    // }

    // ssize_t recv(int sockfd, void *buf, size_t len, int flags)
    // {
    //     if (!lon::util::HookState::isEnable())
    //     {
    //         return recv_f(sockfd, buf, len, flags);
    //     }
    //     return 0;
    // }

    // ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr,
    //                  socklen_t *addrlen)
    // {
    //     if (!lon::util::HookState::isEnable())
    //     {
    //         return recvfrom_f(sockfd, buf, len, flags, src_addr, addrlen);
    //     }
    //     return 0;
    // }

    // ssize_t write(int fd, const void *buf, size_t count)
    // {
    //     if (!lon::util::HookState::isEnable())
    //     {
    //         return write_f(fd, buf, count);
    //     }
    //     return 0;
    // }

    // ssize_t writev(int fd, const struct iovec *iov, int iovcnt)
    // {
    //     if (!lon::util::HookState::isEnable())
    //     {
    //         return writev_f(fd, iov, iovcnt);
    //     }
    //     return 0;
    // }

    // ssize_t send(int sockfd, const void *buf, size_t len, int flags)
    // {
    //     if (!lon::util::HookState::isEnable())
    //     {
    //         return send_f(sockfd, buf, len, flags);
    //     }
    //     return 0;
    // }

    // ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
    //                const struct sockaddr *dest_addr, socklen_t addrlen)
    // {
    //     if (!lon::util::HookState::isEnable())
    //     {
    //         return sendto_f(sockfd, buf, len, flags, dest_addr, addrlen);
    //     }
    //     return 0;
    // }

    // ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags)
    // {
    //     if (!lon::util::HookState::isEnable())
    //     {
    //         return sendmsg_f(sockfd, msg, flags);
    //     }
    //     return 0;
    // }

    // int close(int fd)
    // {
    //     if (!lon::util::HookState::isEnable())
    //     {
    //         return close_f(fd);
    //     }
    //     return 0;
    // }

    // int fcntl(int fd, int cmd, ... /* arg */)
    // {
    //     if (!lon::util::HookState::isEnable())
    //     {
    //         return fcntl_f(fd, cmd, ...);
    //     }
    //     return 0;
    // }

    // int ioctl(int fd, unsigned long request, ...)
    // {
    //     if (!lon::util::HookState::isEnable())
    //     {
    //         return ioctl_f(fd, request, ...);
    //     }
    //     return 0;
    // }

    // int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen)
    // {
    //     if (!lon::util::HookState::isEnable())
    //     {
    //         return getsockopt_f(sockfd, level, optname, optval, optlen);
    //     }
    //     return 0;
    // }

    // int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
    // {
    //     if (!lon::util::HookState::isEnable())
    //     {
    //         return setsockopt_f(sockfd, level, optname, optval, optlen);
    //     }
    //     return 0;
    // }
}
