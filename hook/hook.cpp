#include "hook/hook.h"

#ifdef _WIN32
#define HOOK_FUN(XX)                                                                               \
    XX(Sleep)                                                                                      \
    XX(socket)                                                                                     \
    XX(connect)                                                                                    \
    XX(accept)                                                                                     \
    XX(recv)                                                                                       \
    XX(WSARecv)                                                                                    \
    XX(recvfrom)                                                                                   \
    XX(WSARecvFrom)                                                                                \
    XX(send)                                                                                       \
    XX(WSASend)                                                                                    \
    XX(sendto)                                                                                     \
    XX(WSASendTo)                                                                                  \
    XX(closesocket)                                                                                \
    XX(ioctlsocket)                                                                                \
    XX(getsockopt)                                                                                 \
    XX(setsockopt)
#else
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
#endif

struct timerinfo
{
    int canceled = 0;
};

/*
 * 	fd 			 	文件描述符
 * 	fun				原始函数
 *	hook_name	    hook的函数名称
 *	event			事件
 *	timeout_type	超时时间类型
 *	args			可变参数
 *
 * 	例如：return io(fd, read_f, "read", lon::scheduler::IOScheduler::Event::READ,
 *                  lon::hook::Fd::TimeoutType::READ, buf, count);
 */
template <typename OrgFun, typename... Args>
static ssize_t io(int fd, OrgFun fun, const char *hook_name,
                  lon::scheduler::IOScheduler::Event event, lon::hook::Fd::TimeoutType timeout_type,
                  Args &&...args)
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
#ifdef _WIN32
    if (ret == SOCKET_ERROR)
    {
        int err = WSAGetLastError();
        if (err != WSAEWOULDBLOCK)
        {
            return -1;
        }
    }
    else
    {
        return ret;
    }
#else
    while (ret == -1 && errno == EINTR)
    {
        ret = fun(fd, std::forward<Args>(args)...);
    }
    if (ret != -1 || errno != EAGAIN)
    {
        return ret;
    }
#endif

    auto ios                         = lon::scheduler::IOScheduler::getThis();
    lon::scheduler::Timer::Ptr timer = nullptr;
    std::weak_ptr<timerinfo> w_tinfo(tinfo);

    if (timeout >= 0)
    {
        timer = ios->addConditionTimer(
            timeout,
            [w_tinfo, fd, ios, event]()
            {
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

        LON_ERROR(LON_LOG_ROOT) << hook_name << "io::addEvent(" << fd << ", " << event << ")";

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

    return ret;
}

extern "C"
{
#ifdef _WIN32
// TODO - 因为现在windows用伪hook，这里直接给赋hook_xx的函数
#define XX(name) name##_fun name##_f = nullptr;
    HOOK_FUN(XX)
#undef XX
    VOID WINAPI hook_Sleep(_In_ DWORD dwMilliseconds)
    {
        // FIXME - 这里因为有其他依赖使用了Sleep，会导致hook嵌套，ioscheduler卡死
        return Sleep_f(dwMilliseconds);
        CHECK_HOOK(Sleep_f, dwMilliseconds)
        auto fiber = lon::fiber::Fiber::getThis();
        auto ios   = lon::scheduler::IOScheduler::getThis();
        ios->addTimer(dwMilliseconds,
                      std::bind((void (lon::scheduler::Scheduler::*)(
                                    lon::fiber::Fiber::Ptr,
                                    int thread))&lon::scheduler::IOScheduler::schedule,
                                ios, fiber, -1),
                      false);
        lon::fiber::Fiber::yieldToHold(lon::scheduler::IOScheduler::getMainFiber());

        return;
    }

    unsigned int sleep(unsigned int seconds)
    {
        if (!lon::util::HookState::isEnable())
        {
            Sleep_f(seconds * 1000);
            return 0;
        }
        auto fiber = lon::fiber::Fiber::getThis();
        auto ios   = lon::scheduler::IOScheduler::getThis();
        ios->addTimer(seconds * 1000,
                      std::bind((void (lon::scheduler::Scheduler::*)(
                                    lon::fiber::Fiber::Ptr,
                                    int thread))&lon::scheduler::IOScheduler::schedule,
                                ios, fiber, -1),
                      false);
        lon::fiber::Fiber::yieldToHold(lon::scheduler::IOScheduler::getMainFiber());

        return 0;
    }

    int usleep(useconds_t usec)
    {
        if (!lon::util::HookState::isEnable())
        {
            Sleep_f(usec / 1000);
            return 0;
        }
        auto fiber = lon::fiber::Fiber::getThis();
        auto ios   = lon::scheduler::IOScheduler::getThis();
        ios->addTimer(usec / 1000,
                      std::bind((void (lon::scheduler::Scheduler::*)(
                                    lon::fiber::Fiber::Ptr,
                                    int thread))&lon::scheduler::IOScheduler::schedule,
                                ios, fiber, -1),
                      false);
        lon::fiber::Fiber::yieldToHold(lon::scheduler::IOScheduler::getMainFiber());

        return 0;
    }

    int nanosleep(const struct timespec *req, struct timespec *rem)
    {
        auto msec = req->tv_sec * 1000 + req->tv_nsec / (1000 * 1000);
        if (!lon::util::HookState::isEnable())
        {
            Sleep_f(msec);
            return 0;
        }
        auto fiber = lon::fiber::Fiber::getThis();
        auto ios   = lon::scheduler::IOScheduler::getThis();
        ios->addTimer(msec,
                      std::bind((void (lon::scheduler::Scheduler::*)(
                                    lon::fiber::Fiber::Ptr,
                                    int thread))&lon::scheduler::IOScheduler::schedule,
                                ios, fiber, -1),
                      false);
        lon::fiber::Fiber::yieldToHold(lon::scheduler::IOScheduler::getMainFiber());

        return 0;
    }
    SOCKET WSAAPI hook_socket(_In_ int af, _In_ int type, _In_ int protocoll)
    {
        CHECK_HOOK(socket_f, af, type, protocoll)
        int fd = socket_f(af, type, protocoll);
        if (fd < 0)
        {
            return fd;
        }
        FDMGR.get(fd, true);
        return fd;
    }

    int connect_with_timeout(int sockfd, const struct sockaddr *addr, socklen_t addrlen,
                             int64_t timeout_ms)
    {
        CHECK_HOOK(connect_f, sockfd, addr, addrlen);
        auto fd_obj = FDMGR.get(sockfd);
        if (fd_obj == nullptr || fd_obj->isClose())
        {
            errno = EBADF;
            return -1;
        }
        if (!fd_obj->isSocket())
        {
            return connect_f(sockfd, addr, addrlen);
        }
        if (fd_obj->getUserNonBlock())
        {
            return connect_f(sockfd, addr, addrlen);
        }

        int ret = connect_f(sockfd, addr, addrlen);
#ifdef _WIN32
        if (ret == SOCKET_ERROR)
        {
            int err = WSAGetLastError();
            if (err == WSAEWOULDBLOCK || err == WSAEINPROGRESS)
            {
                errno = EINPROGRESS;
            }
            else
            {
                return ret;
            }
        }
#endif
        if (ret == 0)
        {
            return 0;
        }
        else if (ret != -1 || errno != EINPROGRESS)
        {
            return ret;
        }

        auto ios                         = lon::scheduler::IOScheduler::getThis();
        lon::scheduler::Timer::Ptr timer = nullptr;
        auto tinfo                       = std::make_shared<timerinfo>();
        std::weak_ptr<timerinfo> w_tinfo(tinfo);

        if (timeout_ms >= 0)
        {
            timer = ios->addConditionTimer(
                timeout_ms,
                [w_tinfo, sockfd, ios]()
                {
                    auto t = w_tinfo.lock();
                    if (!t || t->canceled)
                    {
                        return;
                    }
                    t->canceled = ETIMEDOUT;
                    ios->cancelEvent(sockfd, lon::scheduler::IOScheduler::Event::WRITE);
                },
                w_tinfo);
        }

        int rt = ios->addEvent(sockfd, lon::scheduler::IOScheduler::Event::WRITE);
        if (rt)
        {
            LON_ERROR(LON_LOG_ROOT) << "connect_with_timeout::addEvent(" << sockfd << ", WRITE)";

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
        }
#ifdef _WIN32
        int error = 0;
        int len   = sizeof(error);
        if (getsockopt_f(sockfd, SOL_SOCKET, SO_ERROR, (char *)&error, &len) == -1)
#else
        char error    = 0;
        socklen_t len = sizeof(int);
        if (getsockopt_f(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) == -1)
#endif
        {
            return -1;
        }
        if (!error)
        {
            return 0;
        }
        else
        {
            errno = error;
            return -1;
        }
    }

    int WSAAPI hook_connect(_In_ SOCKET s,
                            _In_reads_bytes_(namelen) const struct sockaddr FAR *name,
                            _In_ int namelen)
    {
        return connect_with_timeout(
            s, name, namelen, lon::config::GlobalConfig::Instance().config_tcp_timeout->getData());
    }

    SOCKET WSAAPI hook_accept(_In_ SOCKET s,
                              _Out_writes_bytes_opt_(*addrlen) struct sockaddr FAR *addr,
                              _Inout_opt_ int FAR *addrlen)
    {
        int fd = io(s, accept_f, "accept", lon::scheduler::IOScheduler::Event::READ,
                    lon::hook::Fd::TimeoutType::READ, addr, addrlen);
        if (fd >= 0)
        {
            FDMGR.get(fd, true);
        }

        return fd;
    }

    int WSAAPI hook_recv(_In_ SOCKET s,
                         _Out_writes_bytes_to_(len, return)
                             __out_data_source(NETWORK) char FAR *buf,
                         _In_ int len, _In_ int flags)
    {
        return io(s, recv_f, "recv", lon::scheduler::IOScheduler::Event::READ,
                  lon::hook::Fd::TimeoutType::READ, buf, len, flags);
    }

    int WSAAPI hook_WSARecv(_In_ SOCKET s,
                            _In_reads_(dwBufferCount) __out_data_source(NETWORK) LPWSABUF lpBuffers,
                            _In_ DWORD dwBufferCount, _Out_opt_ LPDWORD lpNumberOfBytesRecvd,
                            _Inout_ LPDWORD lpFlags, _Inout_opt_ LPWSAOVERLAPPED lpOverlapped,
                            _In_opt_ LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
    {
        return io(s, WSARecv_f, "WSARecv", lon::scheduler::IOScheduler::Event::READ,
                  lon::hook::Fd::TimeoutType::READ, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd,
                  lpFlags, lpOverlapped, lpCompletionRoutine);
    }

    int WSAAPI hook_recvfrom(
        _In_ SOCKET s, _Out_writes_bytes_to_(len, return) __out_data_source(NETWORK) char FAR *buf,
        _In_ int len, _In_ int flags,
        _Out_writes_bytes_to_opt_(*fromlen, *fromlen) struct sockaddr FAR *from,
        _Inout_opt_ int FAR *fromlen)
    {
        return io(s, recvfrom_f, "recvfrom", lon::scheduler::IOScheduler::Event::READ,
                  lon::hook::Fd::TimeoutType::READ, buf, len, flags, from, fromlen);
    }

    int WSAAPI hook_WSARecvFrom(
        _In_ SOCKET s, _In_reads_(dwBufferCount) __out_data_source(NETWORK) LPWSABUF lpBuffers,
        _In_ DWORD dwBufferCount, _Out_opt_ LPDWORD lpNumberOfBytesRecvd, _Inout_ LPDWORD lpFlags,
        _Out_writes_bytes_to_opt_(*lpFromlen, *lpFromlen) struct sockaddr FAR *lpFrom,
        _Inout_opt_ LPINT lpFromlen, _Inout_opt_ LPWSAOVERLAPPED lpOverlapped,
        _In_opt_ LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
    {
        return io(s, WSARecvFrom_f, "WSARecvFrom", lon::scheduler::IOScheduler::Event::READ,
                  lon::hook::Fd::TimeoutType::READ, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd,
                  lpFlags, lpFrom, lpFromlen, lpOverlapped, lpCompletionRoutine);
    }

    int WSAAPI hook_send(_In_ SOCKET s, _In_reads_bytes_(len) const char FAR *buf, _In_ int len,
                         _In_ int flags)
    {
        return io(s, send_f, "send", lon::scheduler::IOScheduler::Event::WRITE,
                  lon::hook::Fd::TimeoutType::WRITE, buf, len, flags);
    }

    int WSAAPI hook_WSASend(_In_ SOCKET s, _In_reads_(dwBufferCount) LPWSABUF lpBuffers,
                            _In_ DWORD dwBufferCount, _Out_opt_ LPDWORD lpNumberOfBytesSent,
                            _In_ DWORD dwFlags, _Inout_opt_ LPWSAOVERLAPPED lpOverlapped,
                            _In_opt_ LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
    {
        return io(s, WSASend_f, "WSASend", lon::scheduler::IOScheduler::Event::WRITE,
                  lon::hook::Fd::TimeoutType::WRITE, lpBuffers, dwBufferCount, lpNumberOfBytesSent,
                  dwFlags, lpOverlapped, lpCompletionRoutine);
    }

    int WSAAPI hook_sendto(_In_ SOCKET s, _In_reads_bytes_(len) const char FAR *buf, _In_ int len,
                           _In_ int flags, _In_reads_bytes_(tolen) const struct sockaddr FAR *to,
                           _In_ int tolen)
    {
        return io(s, sendto_f, "sendto", lon::scheduler::IOScheduler::Event::WRITE,
                  lon::hook::Fd::TimeoutType::WRITE, buf, len, flags, to, tolen);
    }

    int WSAAPI hook_WSASendTo(_In_ SOCKET s, _In_reads_(dwBufferCount) LPWSABUF lpBuffers,
                              _In_ DWORD dwBufferCount, _Out_opt_ LPDWORD lpNumberOfBytesSent,
                              _In_ DWORD dwFlags,
                              _In_reads_bytes_opt_(iTolen) const struct sockaddr FAR *lpTo,
                              _In_ int iTolen, _Inout_opt_ LPWSAOVERLAPPED lpOverlapped,
                              _In_opt_ LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
    {
        return io(s, WSASendTo_f, "WSASendTo", lon::scheduler::IOScheduler::Event::WRITE,
                  lon::hook::Fd::TimeoutType::WRITE, lpBuffers, dwBufferCount, lpNumberOfBytesSent,
                  dwFlags, lpTo, iTolen, lpOverlapped, lpCompletionRoutine);
    }

    int WSAAPI hook_closesocket(_In_ SOCKET s)
    {
        CHECK_HOOK(closesocket_f, s)
        auto fd_obj = FDMGR.get(s);
        if (fd_obj)
        {
            auto ios = lon::scheduler::IOScheduler::getThis();
            ios->cancelAll(s);
            FDMGR.del(s);
        }
        return closesocket_f(s);
    }

    int WSAAPI hook_ioctlsocket(_In_ SOCKET s, _In_ long cmd,
                                _When_(cmd != FIONREAD, _Inout_) _When_(cmd == FIONREAD, _Out_)
                                    u_long FAR *argp)
    {
        CHECK_HOOK(ioctlsocket_f, s, cmd, argp)
        if (cmd == FIONBIO)
        {
            bool is_user_nonblock = (*argp != 0);

            auto fd_obj = FDMGR.get((int)s);
            if (fd_obj && fd_obj->isSocket() && !fd_obj->isClose())
            {
                fd_obj->setUserNonBlock(is_user_nonblock);
                u_long sys_nonblock = 1;
                return ioctlsocket_f(s, FIONBIO, &sys_nonblock);
            }
        }

        return ioctlsocket_f(s, cmd, argp);
    }

    int WSAAPI hook_getsockopt(_In_ SOCKET s, _In_ int level, _In_ int optname,
                               _Out_writes_bytes_(*optlen) char FAR *optval,
                               _Inout_ int FAR *optlen)
    {
        return getsockopt_f(s, level, optname, optval, optlen);
    }

    int WSAAPI hook_setsockopt(_In_ SOCKET s, _In_ int level, _In_ int optname,
                               _In_reads_bytes_opt_(optlen) const char FAR *optval, _In_ int optlen)
    {
        CHECK_HOOK(setsockopt_f, s, level, optname, optval, optlen);
        if (level == SOL_SOCKET)
        {
            if (optname == SO_RCVTIMEO || optname == SO_SNDTIMEO)
            {
                auto fd_obj = FDMGR.get(s);
                if (fd_obj)
                {
                    const timeval *tv = (const timeval *)optval;
                    lon::hook::Fd::TimeoutType type =
                        (optname == SO_RCVTIMEO ? lon::hook::Fd::TimeoutType::READ
                                                : lon::hook::Fd::TimeoutType::WRITE);
                    fd_obj->setTimeout(type, tv->tv_sec * 1000 + tv->tv_usec / 1000);
                }
            }
        }
        return setsockopt_f(s, level, optname, optval, optlen);
    }
#else _WIN32
#define XX(name) name##_fun name##_f = nullptr;
    HOOK_FUN(XX)
#undef XX

    unsigned int sleep(unsigned int seconds)
    {
        CHECK_HOOK(sleep_f, seconds)
        auto fiber = lon::fiber::Fiber::getThis();
        auto ios   = lon::scheduler::IOScheduler::getThis();
        ios->addTimer(seconds * 1000,
                      std::bind((void (lon::scheduler::Scheduler::*)(
                                    lon::fiber::Fiber::Ptr,
                                    int thread))&lon::scheduler::IOScheduler::schedule,
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
        ios->addTimer(usec / 1000,
                      std::bind((void (lon::scheduler::Scheduler::*)(
                                    lon::fiber::Fiber::Ptr,
                                    int thread))&lon::scheduler::IOScheduler::schedule,
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
        ios->addTimer(msec,
                      std::bind((void (lon::scheduler::Scheduler::*)(
                                    lon::fiber::Fiber::Ptr,
                                    int thread))&lon::scheduler::IOScheduler::schedule,
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

    int connect_with_timeout(int sockfd, const struct sockaddr *addr, socklen_t addrlen,
                             int64_t timeout_ms)
    {
        CHECK_HOOK(connect_f, sockfd, addr, addrlen);
        auto fd_obj = FDMGR.get(sockfd);
        if (fd_obj == nullptr || fd_obj->isClose())
        {
            errno = EBADF;
            return -1;
        }
        if (!fd_obj->isSocket())
        {
            return connect_f(sockfd, addr, addrlen);
        }
        if (fd_obj->getUserNonBlock())
        {
            return connect_f(sockfd, addr, addrlen);
        }

        int ret = connect_f(sockfd, addr, addrlen);
        if (ret == 0)
        {
            return 0;
        }
        else if (ret != -1 || errno != EINPROGRESS)
        {
            return ret;
        }

        auto ios                         = lon::scheduler::IOScheduler::getThis();
        lon::scheduler::Timer::Ptr timer = nullptr;
        auto tinfo                       = std::make_shared<timerinfo>();
        std::weak_ptr<timerinfo> w_tinfo(tinfo);

        if (timeout_ms >= 0)
        {
            timer = ios->addConditionTimer(
                timeout_ms,
                [w_tinfo, sockfd, ios]()
                {
                    auto t = w_tinfo.lock();
                    if (!t || t->canceled)
                    {
                        return;
                    }
                    t->canceled = ETIMEDOUT;
                    ios->cancelEvent(sockfd, lon::scheduler::IOScheduler::Event::WRITE);
                },
                w_tinfo);
        }

        int rt = ios->addEvent(sockfd, lon::scheduler::IOScheduler::Event::WRITE);
        if (rt)
        {
            LON_ERROR(LON_LOG_ROOT) << "connect_with_timeout::addEvent(" << sockfd << ", WRITE)";

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
        }
        int error     = 0;
        socklen_t len = sizeof(int);
        if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) == -1)
        {
            return -1;
        }
        if (!error)
        {
            return 0;
        }
        else
        {
            errno = error;
            return -1;
        }
    }

    int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
    {
        return connect_with_timeout(
            sockfd, addr, addrlen,
            lon::config::GlobalConfig::Instance().config_tcp_timeout->getData());
    }

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
        auto fd_obj = FDMGR.get(fd);
        if (fd_obj)
        {
            auto ios = lon::scheduler::IOScheduler::getThis();
            ios->cancelAll(fd);
            FDMGR.del(fd);
        }
        return close_f(fd);
    }

    int fcntl(int fd, int cmd, ... /* arg */)
    {
        va_list args;
        va_start(args, cmd);
        switch (cmd)
        {
        case F_SETFL:
        {
            int arg = va_arg(args, int);
            va_end(args);
            auto fd_obj = FDMGR.get(fd);
            if (fd_obj == nullptr || !fd_obj->isSocket() || fd_obj->isClose())
            {
                return fcntl_f(fd, cmd, arg);
            }
            fd_obj->setUserNonBlock(arg & O_NONBLOCK);
            if (fd_obj->getSysNonBlock())
            {
                arg |= O_NONBLOCK;
            }
            else
            {
                arg &= ~O_NONBLOCK;
            }
            return fcntl_f(fd, cmd, arg);
        }
        break;
        case F_GETFL:
        {
            va_end(args);
            int arg     = fcntl_f(fd, cmd);
            auto fd_obj = FDMGR.get(fd);
            if (fd_obj == nullptr || !fd_obj->isSocket() || fd_obj->isClose())
            {
                return arg;
            }
            if (fd_obj->getUserNonBlock())
            {
                return arg | O_NONBLOCK;
            }
            else
            {
                return arg & ~O_NONBLOCK;
            }
        }
        break;
        case F_DUPFD:
        case F_DUPFD_CLOEXEC:
        case F_SETFD:
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

    int ioctl(int fd, unsigned long request, ...)
    {
        va_list args;
        va_start(args, request);
        void *arg = va_arg(args, void *);
        va_end(args);

        if (request == FIONBIO)
        {
            bool is_user_nonblock = !!*(int *)arg;
            auto fd_obj           = FDMGR.get(fd);
            if (fd_obj == nullptr || !fd_obj->isSocket() || fd_obj->isClose())
            {
                return ioctl_f(fd, request, arg);
            }
            fd_obj->setUserNonBlock(is_user_nonblock);
        }
        return ioctl_f(fd, request, arg);
    }

    int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen)
    {
        return getsockopt_f(sockfd, level, optname, optval, optlen);
    }

    int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
    {
        CHECK_HOOK(setsockopt_f, sockfd, level, optname, optval, optlen);
        if (level == SOL_SOCKET)
        {
            if (optname == SO_RCVTIMEO || optname == SO_SNDTIMEO)
            {
                auto fd_obj = FDMGR.get(sockfd);
                if (fd_obj)
                {
                    const timeval *tv = (const timeval *)optval;
                    lon::hook::Fd::TimeoutType type =
                        (optname == SO_RCVTIMEO ? lon::hook::Fd::TimeoutType::READ
                                                : lon::hook::Fd::TimeoutType::WRITE);
                    fd_obj->setTimeout(type, tv->tv_sec * 1000 + tv->tv_usec / 1000);
                }
            }
        }
        return setsockopt_f(sockfd, level, optname, optval, optlen);
    }
#endif
}

namespace lon
{
namespace hook
{
Hook::Hook()
{
    static int is_inited = false;
    if (is_inited)
    {
        return;
    }
#ifdef _WIN32
    MH_Initialize();
    WSADATA wsa;
    LON_ASSERT(WSAStartup(MAKEWORD(2, 2), &wsa) == 0);
#define XX(name)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        auto resolve_dll = [](const char *name) -> const char *                                    \
        {                                                                                          \
            if (strcmp(name, "Sleep") == 0)                                                        \
            {                                                                                      \
                return "KernelBase.dll";                                                           \
            }                                                                                      \
            return "ws2_32.dll";                                                                   \
        };                                                                                         \
        const char *dll = resolve_dll(#name);                                                      \
        HMODULE h       = GetModuleHandleA(dll);                                                   \
        if (!h)                                                                                    \
            h = LoadLibraryA(dll);                                                                 \
        FARPROC p = GetProcAddress(h, #name);                                                      \
        LON_ASSERT(p);                                                                             \
                                                                                                   \
        auto st =                                                                                  \
            MH_CreateHook(reinterpret_cast<LPVOID>(p), reinterpret_cast<LPVOID>(&hook_##name),     \
                          reinterpret_cast<LPVOID *>(&name##_f));                                  \
        LON_ASSERT(st == MH_OK);                                                                   \
                                                                                                   \
        st = MH_EnableHook(reinterpret_cast<LPVOID>(p));                                           \
        LON_ASSERT(st == MH_OK);                                                                   \
    } while (0);
    HOOK_FUN(XX)
#undef XX
#else
#define XX(name) name##_f = (name##_fun)dlsym(RTLD_NEXT, #name);
    HOOK_FUN(XX)
#undef XX
#endif
}

Hook::~Hook()
{
#ifdef _WIN32
    WSACleanup();
#endif
}

Hook &Hook::Instance()
{
    static Hook instance;
    return instance;
}

void Hook::enable() { util::HookState::enable(); }

void Hook::disable() { util::HookState::disable(); }

} // namespace hook
} // namespace lon
