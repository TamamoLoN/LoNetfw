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
    XX(recvmsg)                                                                                    \
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

struct timerinfo
{
    int canceled = 0;
};

template <typename OrgFun, typename... Args>
static ssize_t io(int fd, OrgFun fun, const char *hook_name,
                  lon::scheduler::IOScheduler::Event event, lon::hook::Fd::TimeoutType timeout_type,
                  Args &&... args)
{
    if (!lon::util::HookState::isEnable())
    {
        return fun(fd, std::forward<Args>(args)...);
    }

    auto fd_obj = FDMGR.get(fd);
    if (fd_obj == nullptr)
    {
        return fun(fd, std::forward<Args>(args)...);
    }

    if (fd_obj->isClose())
    {
        errno = EBADF;
        return -1;
    }

    if (!fd_obj->isSocket() || fd_obj->getUserNonBlock())
    {
        return fun(fd, std::forward<Args>(args)...);
    }

    auto timeout = fd_obj->getTimeout(timeout_type);
    auto tinfo   = std::make_shared<timerinfo>();

retry:
    ssize_t ret = fun(fd, std::forward<Args>(args)...);
    while (ret == -1 && errno == EINTR)
    {
        ret = fun(fd, std::forward<Args>(args)...);
    }
    if (ret == -1 && errno == EAGAIN)
    {
        auto ios                         = lon::scheduler::IOScheduler::getThis();
        lon::scheduler::Timer::Ptr timer = nullptr;
        std::weak_ptr<timerinfo> w_tinfo(tinfo);

        if (timeout != -1)
        {
            timer = ios->addConditionTimer(
                timeout,
                [w_tinfo, fd, ios, event]() {
                    auto t = w_tinfo.lock();
                    if (!t || t->canceled)
                    {
                        return;
                    }
                    t->canceled = ETIMEDOUT;
                    ios->cancelEvent(fd, event);
                },
                w_tinfo);
        }

        int rt = ios->addEvent(fd, event);
        if (rt)
        {

            LON_ERROR(LON_LOG_ROOT) << hook_name << "addEvent(" << fd << ", " << event << ")";

            if (timer)
            {
                timer->cancel();
            }
            return -1;
        }
        else
        {
            lon::fiber::Fiber::yieldToHold(lon::scheduler::IOScheduler::getMainFiber());
            if (timer)
            {
                timer->cancel();
            }
            if (tinfo->canceled)
            {
                errno = tinfo->canceled;
                return -1;
            }
            goto retry;
        }
    }
    return ret;
}

extern "C"
{
#define XX(name) name##_fun name##_f = nullptr;
    HOOK_FUN(XX)
#undef XX

    unsigned int sleep(unsigned int seconds)
    {
        CHECK_HOOK(sleep_f, seconds)
        auto fiber = lon::fiber::Fiber::getThis();
        auto ios   = lon::scheduler::IOScheduler::getThis();
        ios->addTimer(
            seconds * 1000,
            std::bind((void (lon::scheduler::Scheduler::*)(lon::fiber::Fiber::Ptr, int thread)) &
                          lon::scheduler::IOScheduler::schedule,
                      ios, fiber, -1),
            false);
        lon::fiber::Fiber::yieldToHold(lon::scheduler::IOScheduler::getMainFiber());

        return 0;
    }

    int usleep(useconds_t usec)
    {
        CHECK_HOOK(usleep_f, usec)
        auto fiber = lon::fiber::Fiber::getThis();
        auto ios   = lon::scheduler::IOScheduler::getThis();
        ios->addTimer(
            usec / 1000,
            std::bind((void (lon::scheduler::Scheduler::*)(lon::fiber::Fiber::Ptr, int thread)) &
                          lon::scheduler::IOScheduler::schedule,
                      ios, fiber, -1),
            false);
        lon::fiber::Fiber::yieldToHold(lon::scheduler::IOScheduler::getMainFiber());

        return 0;
    }

    int nanosleep(const struct timespec *req, struct timespec *rem)
    {
        CHECK_HOOK(nanosleep_f, req, rem)
        auto msec  = req->tv_sec * 1000 + req->tv_nsec / (1000 * 1000);
        auto fiber = lon::fiber::Fiber::getThis();
        auto ios   = lon::scheduler::IOScheduler::getThis();
        ios->addTimer(
            msec,
            std::bind((void (lon::scheduler::Scheduler::*)(lon::fiber::Fiber::Ptr, int thread)) &
                          lon::scheduler::IOScheduler::schedule,
                      ios, fiber, -1),
            false);
        lon::fiber::Fiber::yieldToHold(lon::scheduler::IOScheduler::getMainFiber());

        return 0;
    }

    int socket(int domain, int type, int protocol)
    {
        CHECK_HOOK(socket_f, domain, type, protocol)
        int fd = socket_f(domain, type, protocol);
        if (fd < 0)
        {
            return fd;
        }
        FDMGR.get(fd, true);
        return fd;
    }

    // int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
    // {
    //     if (!lon::util::HookState::isEnable())
    //     {
    //         return connect_f(sockfd, addr, addrlen);
    //     }
    //     return 0;
    // }

    int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
    {
        int fd = io(sockfd, accept_f, "accept", lon::scheduler::IOScheduler::Event::READ,
                    lon::hook::Fd::TimeoutType::READ, addr, addrlen);
        if (fd >= 0)
        {
            FDMGR.get(fd, true);
        }

        return fd;
    }

    // int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
    // {
    //     if (!lon::util::HookState::isEnable())
    //     {
    //         return bind_f(sockfd, addr, addrlen);
    //     }
    //     return 0;
    // }

    ssize_t read(int fd, void *buf, size_t count)
    {
        return io(fd, read_f, "read", lon::scheduler::IOScheduler::Event::READ,
                  lon::hook::Fd::TimeoutType::READ, buf, count);
    }

    ssize_t readv(int fd, const struct iovec *iov, int iovcnt)
    {
        return io(fd, readv_f, "readv", lon::scheduler::IOScheduler::Event::READ,
                  lon::hook::Fd::TimeoutType::READ, iov, iovcnt);
    }

    ssize_t recv(int sockfd, void *buf, size_t len, int flags)
    {
        return io(sockfd, recv_f, "recv", lon::scheduler::IOScheduler::Event::READ,
                  lon::hook::Fd::TimeoutType::READ, buf, len, flags);
    }

    ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr,
                     socklen_t *addrlen)
    {
        return io(sockfd, recvfrom_f, "recvfrom", lon::scheduler::IOScheduler::Event::READ,
                  lon::hook::Fd::TimeoutType::READ, buf, len, flags, src_addr, addrlen);
    }

    ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags)
    {
        return io(sockfd, recvmsg_f, "recvmsg", lon::scheduler::IOScheduler::Event::READ,
                  lon::hook::Fd::TimeoutType::READ, msg, flags);
    }

    ssize_t write(int fd, const void *buf, size_t count)
    {
        return io(fd, write_f, "write", lon::scheduler::IOScheduler::Event::WRITE,
                  lon::hook::Fd::TimeoutType::WRITE, buf, count);
    }

    ssize_t writev(int fd, const struct iovec *iov, int iovcnt)
    {
        return io(fd, writev_f, "writev", lon::scheduler::IOScheduler::Event::WRITE,
                  lon::hook::Fd::TimeoutType::WRITE, iov, iovcnt);
    }

    ssize_t send(int sockfd, const void *buf, size_t len, int flags)
    {
        return io(sockfd, send_f, "send", lon::scheduler::IOScheduler::Event::WRITE,
                  lon::hook::Fd::TimeoutType::WRITE, buf, len, flags);
    }

    ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
                   const struct sockaddr *dest_addr, socklen_t addrlen)
    {
        return io(sockfd, sendto_f, "sendto", lon::scheduler::IOScheduler::Event::WRITE,
                  lon::hook::Fd::TimeoutType::WRITE, buf, len, flags, dest_addr, addrlen);
    }

    ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags)
    {
        return io(sockfd, sendmsg_f, "sendmsg", lon::scheduler::IOScheduler::Event::WRITE,
                  lon::hook::Fd::TimeoutType::WRITE, msg, flags);
    }

    int close(int fd)
    {
        CHECK_HOOK(close_f, fd)
        auto fd_ptr = FDMGR.get(fd);
        if (fd_ptr)
        {
            auto ios = lon::scheduler::IOScheduler::getThis();
            ios->cancelAll(fd);
            FDMGR.del(fd);
        }
        return close_f(fd);
    }

    int fcntl(int fd, int cmd, ... /* arg */)
    {
        // CHECK_HOOK(fcntl_f, fd, cmd, )
        va_list args;
        va_start(args, cmd);
        switch (cmd)
        {
        case F_DUPFD:
        case F_DUPFD_CLOEXEC:
        case F_SETFD:
        case F_SETFL:
        case F_SETOWN:
        case F_SETSIG:
        case F_SETLEASE:
        case F_NOTIFY:
        case F_SETPIPE_SZ:
        {
            int arg = va_arg(args, int);
            va_end(args);
            return fcntl_f(fd, cmd, arg);
        }
        break;
        case F_GETFD:
        case F_GETFL:
        case F_GETOWN:
        case F_GETSIG:
        case F_GETLEASE:
        case F_GETPIPE_SZ:
        {
            va_end(args);
            return fcntl_f(fd, cmd);
        }
        break;
        case F_SETLK:
        case F_SETLKW:
        case F_GETLK:
        {
            va_end(args);
            struct flock *arg = va_arg(args, struct flock *);
            return fcntl_f(fd, cmd, arg);
        }
        break;
        case F_GETOWN_EX:
        case F_SETOWN_EX:
        {
            va_end(args);
            struct f_owner_exlock *arg = va_arg(args, struct f_owner_exlock *);
            return fcntl_f(fd, cmd, arg);
        }
        break;
        default:
        {
            va_end(args);
            return fcntl_f(fd, cmd);
        }
        break;
        }
    }

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
