#pragma once

#include "config/config.h"
#include "hook/fd.h"
#include "scheduler/ioscheduler.h"
#include "util/util.h"
#include <fcntl.h>
#include <sys/types.h>
#ifdef _WIN32
#include <MinHook.h>
#else
#include <dlfcn.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>
#endif

namespace lon
{
namespace hook
{
class Hook
{
  public:
    Hook();
    ~Hook();
    static Hook &Instance();
    static void enable();
    static void disable();
};
static auto s_hook = Hook::Instance();

} // namespace hook
} // namespace lon

extern "C"
{
#define CHECK_HOOK(funptr, ...)                                                                    \
    if (!lon::util::HookState::isEnable())                                                         \
    {                                                                                              \
        return funptr(__VA_ARGS__);                                                                \
    }
#ifdef _WIN32
#define useconds_t unsigned int
    // sleep相关api
    typedef VOID(WINAPI *Sleep_fun)(_In_ DWORD dwMilliseconds);
    extern Sleep_fun Sleep_f;
    extern unsigned int sleep(unsigned int seconds);
    extern int usleep(useconds_t usec);
    extern int nanosleep(const struct timespec *req, struct timespec *rem);

    // socket相关api
    typedef SOCKET(WSAAPI *socket_fun)(_In_ int af, _In_ int type, _In_ int protocoll);
    extern socket_fun socket_f;

    typedef int(WSAAPI *connect_fun)(_In_ SOCKET s,
                                     _In_reads_bytes_(namelen) const struct sockaddr FAR *name,
                                     _In_ int namelen);
    extern int connect_with_timeout(int sockfd, const struct sockaddr *addr, socklen_t addrlen,
                                    int64_t timeout_ms);
    extern connect_fun connect_f;

    typedef SOCKET(WSAAPI *accept_fun)(_In_ SOCKET s,
                                       _Out_writes_bytes_opt_(*addrlen) struct sockaddr FAR *addr,
                                       _Inout_opt_ int FAR *addrlen);
    extern accept_fun accept_f;

    // read相关api
    typedef int(WSAAPI *recv_fun)(_In_ SOCKET s,
                                  _Out_writes_bytes_to_(len, return)
                                      __out_data_source(NETWORK) char FAR *buf,
                                  _In_ int len, _In_ int flags);
    extern recv_fun recv_f;

    typedef int(WSAAPI *WSARecv_fun)(
        _In_ SOCKET s, _In_reads_(dwBufferCount) __out_data_source(NETWORK) LPWSABUF lpBuffers,
        _In_ DWORD dwBufferCount, _Out_opt_ LPDWORD lpNumberOfBytesRecvd, _Inout_ LPDWORD lpFlags,
        _Inout_opt_ LPWSAOVERLAPPED lpOverlapped,
        _In_opt_ LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
    extern WSARecv_fun WSARecv_f;

    typedef int(WSAAPI *recvfrom_fun)(
        _In_ SOCKET s, _Out_writes_bytes_to_(len, return) __out_data_source(NETWORK) char FAR *buf,
        _In_ int len, _In_ int flags,
        _Out_writes_bytes_to_opt_(*fromlen, *fromlen) struct sockaddr FAR *from,
        _Inout_opt_ int FAR *fromlen);
    extern recvfrom_fun recvfrom_f;

    typedef int(WSAAPI *WSARecvFrom_fun)(
        _In_ SOCKET s, _In_reads_(dwBufferCount) __out_data_source(NETWORK) LPWSABUF lpBuffers,
        _In_ DWORD dwBufferCount, _Out_opt_ LPDWORD lpNumberOfBytesRecvd, _Inout_ LPDWORD lpFlags,
        _Out_writes_bytes_to_opt_(*lpFromlen, *lpFromlen) struct sockaddr FAR *lpFrom,
        _Inout_opt_ LPINT lpFromlen, _Inout_opt_ LPWSAOVERLAPPED lpOverlapped,
        _In_opt_ LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
    extern WSARecvFrom_fun WSARecvFrom_f;

    typedef int(WSAAPI *send_fun)(_In_ SOCKET s, _In_reads_bytes_(len) const char FAR *buf,
                                  _In_ int len, _In_ int flags);
    extern send_fun send_f;

    typedef int(WSAAPI *WSASend_fun)(
        _In_ SOCKET s, _In_reads_(dwBufferCount) LPWSABUF lpBuffers, _In_ DWORD dwBufferCount,
        _Out_opt_ LPDWORD lpNumberOfBytesSent, _In_ DWORD dwFlags,
        _Inout_opt_ LPWSAOVERLAPPED lpOverlapped,
        _In_opt_ LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
    extern WSASend_fun WSASend_f;

    typedef int(WSAAPI *sendto_fun)(_In_ SOCKET s, _In_reads_bytes_(len) const char FAR *buf,
                                    _In_ int len, _In_ int flags,
                                    _In_reads_bytes_(tolen) const struct sockaddr FAR *to,
                                    _In_ int tolen);
    extern sendto_fun sendto_f;

    typedef int(WSAAPI *WSASendTo_fun)(
        _In_ SOCKET s, _In_reads_(dwBufferCount) LPWSABUF lpBuffers, _In_ DWORD dwBufferCount,
        _Out_opt_ LPDWORD lpNumberOfBytesSent, _In_ DWORD dwFlags,
        _In_reads_bytes_opt_(iTolen) const struct sockaddr FAR *lpTo, _In_ int iTolen,
        _Inout_opt_ LPWSAOVERLAPPED lpOverlapped,
        _In_opt_ LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
    extern WSASendTo_fun WSASendTo_f;

    // fd control api
    typedef int(WSAAPI *closesocket_fun)(_In_ SOCKET s);
    extern closesocket_fun closesocket_f;

    typedef int(WSAAPI *ioctlsocket_fun)(_In_ SOCKET s, _In_ long cmd,
                                         _When_(cmd != FIONREAD, _Inout_)
                                             _When_(cmd == FIONREAD, _Out_) u_long FAR *argp);
    extern ioctlsocket_fun ioctlsocket_f;

    typedef int(WSAAPI *getsockopt_fun)(_In_ SOCKET s, _In_ int level, _In_ int optname,
                                        _Out_writes_bytes_(*optlen) char FAR *optval,
                                        _Inout_ int FAR *optlen);
    extern getsockopt_fun getsockopt_f;

    typedef int(WSAAPI *setsockopt_fun)(_In_ SOCKET s, _In_ int level, _In_ int optname,
                                        _In_reads_bytes_opt_(optlen) const char FAR *optval,
                                        _In_ int optlen);
    extern setsockopt_fun setsockopt_f;
#else
    // sleep相关api
    typedef unsigned int (*sleep_fun)(unsigned int seconds);
    extern sleep_fun sleep_f;

    typedef int (*usleep_fun)(useconds_t usec);
    extern usleep_fun usleep_f;

    typedef int (*nanosleep_fun)(const struct timespec *req, struct timespec *rem);
    extern nanosleep_fun nanosleep_f;

    // socket相关api
    typedef int (*socket_fun)(int domain, int type, int protocol);
    extern socket_fun socket_f;

    typedef int (*connect_fun)(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    extern int connect_with_timeout(int sockfd, const struct sockaddr *addr, socklen_t addrlen,
                                    int64_t timeout_ms);
    extern connect_fun connect_f;

    typedef int (*accept_fun)(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
    extern accept_fun accept_f;

    typedef int (*bind_fun)(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    extern bind_fun bind_f;

    // read相关api
    typedef ssize_t (*read_fun)(int fd, void *buf, size_t count);
    extern read_fun read_f;

    typedef ssize_t (*readv_fun)(int fd, const struct iovec *iov, int iovcnt);
    extern readv_fun readv_f;

    typedef ssize_t (*recv_fun)(int sockfd, void *buf, size_t len, int flags);
    extern recv_fun recv_f;

    typedef ssize_t (*recvfrom_fun)(int sockfd, void *buf, size_t len, int flags,
                                    struct sockaddr *src_addr, socklen_t *addrlen);
    extern recvfrom_fun recvfrom_f;

    typedef ssize_t (*recvmsg_fun)(int sockfd, struct msghdr *msg, int flags);
    extern recvmsg_fun recvmsg_f;

    // write相关api
    typedef ssize_t (*write_fun)(int fd, const void *buf, size_t count);
    extern write_fun write_f;

    typedef ssize_t (*writev_fun)(int fd, const struct iovec *iov, int iovcnt);
    extern writev_fun writev_f;

    typedef ssize_t (*send_fun)(int sockfd, const void *buf, size_t len, int flags);
    extern send_fun send_f;

    typedef ssize_t (*sendto_fun)(int sockfd, const void *buf, size_t len, int flags,
                                  const struct sockaddr *dest_addr, socklen_t addrlen);
    extern sendto_fun sendto_f;

    typedef ssize_t (*sendmsg_fun)(int sockfd, const struct msghdr *msg, int flags);
    extern sendmsg_fun sendmsg_f;

    // fd control api
    typedef int (*close_fun)(int fd);
    extern close_fun close_f;

    typedef int (*fcntl_fun)(int fd, int cmd, ...);
    extern fcntl_fun fcntl_f;

    typedef int (*ioctl_fun)(int fd, unsigned long request, ...);
    extern ioctl_fun ioctl_f;

    typedef int (*getsockopt_fun)(int sockfd, int level, int optname, void *optval,
                                  socklen_t *optlen);
    extern getsockopt_fun getsockopt_f;

    typedef int (*setsockopt_fun)(int sockfd, int level, int optname, const void *optval,
                                  socklen_t optlen);
    extern setsockopt_fun setsockopt_f;
#endif
}
