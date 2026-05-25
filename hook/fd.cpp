#include "hook/fd.h"
#ifdef _WIN32
#include "hook/hook.h"
#endif

#ifdef _WIN32
static int S_ISSOCK(SOCKET s)
{
    HANDLE h = (HANDLE)s;
    DWORD ft = GetFileType(h);
    if (ft != FILE_TYPE_PIPE)
    {
        // 磁盘文件 / 字符设备 / 无效
        return false;
    }
    // PIPE 里可能是：命名管道 / 匿名管道 / socket
    int type = 0;
    int len  = sizeof(type);
    // 只查属性，不收发数据，不阻塞
    if (getsockopt_f(s, SOL_SOCKET, SO_TYPE, (char *)&type, &len) != 0)
    {
        return false; // pipe，但不是 socket
    }
    return true;
}
#endif

namespace lon
{
namespace hook
{
typedef int (*fd_fcntl_fun)(int fd, int cmd, ...);
static fd_fcntl_fun fd_fcntl_f = nullptr;

hook::Fd::Fd(int fd)
    : m_fd(fd), m_is_init(false), m_is_socket(false), m_is_sys_nonblock(false),
      m_is_user_nonblock(false), m_is_closed(true), m_recv_timeout(-1), m_send_timeout(-1)
//   ,m_ioscheculer(nullptr)
{
    init();
}

bool hook::Fd::init()
{
#ifdef _WIN32
    if (m_is_init)
    {
        return true;
    }
    m_recv_timeout = -1;
    m_send_timeout = -1;
    if (!S_ISSOCK((SOCKET)m_fd))
    {
        m_is_init   = false;
        m_is_socket = false;
        return false;
    }
    m_is_init   = true;
    m_is_socket = true;
    // 系统级非阻塞
    u_long nonblock = 1;
    int rc          = ioctlsocket_f((SOCKET)m_fd, FIONBIO, &nonblock);
    LON_ASSERT(rc == 0);

    m_is_sys_nonblock  = true;
    m_is_user_nonblock = false;
    m_is_closed        = false;
    return m_is_init;
#else
    if (fd_fcntl_f == nullptr)
    {
        fd_fcntl_f = (fd_fcntl_fun)dlsym(RTLD_NEXT, "fcntl");
    }
    if (m_is_init)
    {
        return true;
    }
    m_recv_timeout = -1;
    m_send_timeout = -1;
    struct stat fd_stat;
    // 是否已经关闭
    if (fstat(m_fd, &fd_stat) == -1)
    {
        m_is_init   = false;
        m_is_socket = false;
        return false;
    }
    else
    {
        m_is_init   = true;
        m_is_socket = S_ISSOCK(fd_stat.st_mode);
    }

    if (m_is_socket)
    {
        int flags = fd_fcntl_f(m_fd, F_GETFL, 0);
        if (!(flags & O_NONBLOCK))
        {
            fd_fcntl_f(m_fd, F_SETFL, flags | O_NONBLOCK);
        }
        m_is_sys_nonblock = true;
    }
    else
    {
        m_is_sys_nonblock = false;
    }

    m_is_user_nonblock = false;
    m_is_closed        = false;
    return m_is_init;
#endif
}

bool hook::Fd::isInit() const { return m_is_init; }

bool hook::Fd::isSocket() const { return m_is_socket; }

bool hook::Fd::isOpen() const { return !m_is_closed; }

bool hook::Fd::isClose() const { return m_is_closed; }

// bool hook::Fd::close() {}

void hook::Fd::setSysNonBlock(bool is_sys_nonblock) { m_is_sys_nonblock = is_sys_nonblock; }

void hook::Fd::setUserNonBlock(bool is_user_nonblock) { m_is_user_nonblock = is_user_nonblock; }

bool hook::Fd::getSysNonBlock() const { return m_is_sys_nonblock; }

bool hook::Fd::getUserNonBlock() const { return m_is_user_nonblock; }

void hook::Fd::setTimeout(const TimeoutType &rs_type, int64_t timeout)
{
    switch (rs_type)
    {
    case TimeoutType::READ:
        m_recv_timeout = timeout;
        break;
    case TimeoutType::WRITE:
        m_send_timeout = timeout;
        break;
    default:
        return;
    }
}

int64_t hook::Fd::getTimeout(const TimeoutType &rs_type) const
{
    switch (rs_type)
    {
    case TimeoutType::READ:
        return m_recv_timeout;
    case TimeoutType::WRITE:
        return m_send_timeout;
    default:
        return 0;
    }
}

FdManager::FdManager(int size) : m_size(size)
{
#ifdef _WIN32
#else
    m_fds.resize(m_size);
#endif
}

FdManager::~FdManager() {}

Fd::Ptr FdManager::get(int fd, bool auto_create)
{
    MutexType::RdLock rlock(m_mutex);
#ifdef _WIN32
    if (m_fds.find(fd) == m_fds.end())
    {
        if (!auto_create)
        {
            return nullptr;
        }
    }
    else
    {
        if (m_fds[fd] || !auto_create)
        {
            return m_fds[fd];
        }
    }
    rlock.unlock();
    MutexType::WrLock wlock(m_mutex);
    auto new_fd = std::make_shared<Fd>(fd);
    if (fd >= m_fds.size())
    {
        m_size = m_fds.size();
    }
    m_fds[fd] = new_fd;
    return new_fd;
#else
    if (fd >= m_size)
    {
        if (!auto_create)
        {
            return nullptr;
        }
    }
    else
    {
        if (m_fds[fd] || !auto_create)
        {
            return m_fds[fd];
        }
    }
    rlock.unlock();
    MutexType::WrLock wlock(m_mutex);
    auto new_fd = std::make_shared<Fd>(fd);
    if (fd >= m_fds.size())
    {
        m_fds.resize(fd * 1.5);
        m_size = m_fds.size();
    }
    m_fds[fd] = new_fd;
    return new_fd;
#endif
}

void FdManager::del(int fd)
{
    MutexType::WrLock lock(m_mutex);
#ifdef _WIN32
    if (m_fds.find(fd) == m_fds.end())
    {
        return;
    }
#else
    if (fd >= m_size)
    {
        return;
    }
#endif
    m_fds[fd].reset();
}

size_t FdManager::size()
{
    MutexType::RdLock lock(m_mutex);
    size_t count = 0;
#ifdef _WIN32
    for (auto &fd : m_fds)
    {
        if (fd.second)
        {
            ++count;
        }
    }
#else
    for (auto &fd : m_fds)
    {
        if (fd)
        {
            ++count;
        }
    }
#endif
    return count;
}

} // namespace hook
} // namespace lon