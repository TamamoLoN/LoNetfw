#include "net/socket.h"

namespace lon
{
namespace net
{

Socket::Socket(int family, int type, int protocol)
    : m_sockfd(-1), m_family(family), m_type(type), m_protocol(protocol), m_is_connected(false),
      m_local_addr(nullptr), m_peer_addr(nullptr)
{
    // TODO: 实现构造函数
}

Socket::~Socket() { close(); }

Socket::Ptr Socket::create(Socket::Family family, Socket::Type type)
{
    switch (family)
    {
    case IPV4:
        switch (type)
        {
        case TCP:
            return std::make_shared<Socket>(IPV4, TCP, 0);
        case UDP:
            return std::make_shared<Socket>(IPV4, UDP, 0);
        default:
            return nullptr;
        }
    case IPV6:
        switch (type)
        {
        case TCP:
            return std::make_shared<Socket>(IPV6, TCP, 0);
        case UDP:
            return std::make_shared<Socket>(IPV6, UDP, 0);
        default:
            return nullptr;
        }
    case UNIX:
        switch (type)
        {
        case TCP:
            return std::make_shared<Socket>(UNIX, TCP, 0);
        case UDP:
            return std::make_shared<Socket>(UNIX, UDP, 0);
        default:
            return nullptr;
        }
    default:
        return nullptr;
    }
}

Socket::Ptr Socket::create(const Address::Ptr &addr, Socket::Type type)
{
    switch (type)
    {
    case TCP:
        return std::make_shared<Socket>(addr->getFamily(), TCP, 0);
        break;
    case UDP:
        return std::make_shared<Socket>(addr->getFamily(), UDP, 0);
        break;
    default:
        return nullptr;
    }
}

int64_t Socket::getSendTimeout() const
{
    hook::Fd::Ptr fd = FDMGR.get(m_sockfd);
    if (fd)
    {
        return fd->getTimeout(hook::Fd::TimeoutType::WRITE);
    }
    return -1;
}

void Socket::setSendTimeout(int64_t timeout)
{
    struct timeval tv;
    tv.tv_sec  = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;
    setOption(SOL_SOCKET, SO_SNDTIMEO, &tv);
}

int64_t Socket::getRecvTimeout() const
{
    hook::Fd::Ptr fd = FDMGR.get(m_sockfd);
    if (fd)
    {
        return fd->getTimeout(hook::Fd::TimeoutType::READ);
    }
    return -1;
}

void Socket::setRecvTimeout(int64_t timeout)
{
    struct timeval tv;
    tv.tv_sec  = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;
    setOption(SOL_SOCKET, SO_RCVTIMEO, &tv);
}

bool Socket::getOption(int level, int optname, void *optval, socklen_t *optlen)
{
    int ret = getsockopt(m_sockfd, level, optname, optval, optlen);
    if (ret)
    {
        LON_ERROR(LON_LOG_ROOT) << "getOption failed: "
                                << "socket=" << m_sockfd << " level=" << level
                                << " optname=" << optname << " ret = " << ret
                                << " errno = " << errno << " errstr=" << strerror(errno);
        return false;
    }
    return true;
}

bool Socket::setOption(int level, int optname, const void *optval, socklen_t optlen)
{
    int ret = setsockopt(m_sockfd, level, optname, optval, optlen);
    if (ret)
    {
        LON_ERROR(LON_LOG_ROOT) << "setOption failed: "
                                << "socket=" << m_sockfd << " level=" << level
                                << " optname=" << optname << " ret = " << ret
                                << " errno = " << errno << " errstr=" << strerror(errno);
        return false;
    }
    return true;
}

Socket::Ptr Socket::accept()
{
    Socket::Ptr socket = std::make_shared<Socket>(m_family, m_type, m_protocol);
    int new_socket     = ::accept(m_sockfd, nullptr, nullptr);
    if (new_socket == -1)
    {
        LON_ERROR(LON_LOG_ROOT) << "accept failed: "
                                << "socket=" << m_sockfd << " new_socket = " << new_socket
                                << " errno = " << errno << " errstr=" << strerror(errno);
        return nullptr;
    }
    if (socket->init(new_socket))
    {
        return socket;
    }
    return nullptr;
}

bool Socket::init(int socketfd)
{
    auto fd = FDMGR.get(socketfd);
    if (fd && fd->isSocket() && !fd->isClose())
    {
        m_sockfd       = socketfd;
        m_is_connected = true;
        initSocket();
        getLocalAddress();
        getPeerAddress();
        return true;
    }

    return false;
}

bool Socket::bind(const Address::Ptr &addr)
{
    if (!isValid())
    {
        newSocket();
        if (LON_UNLIKELY(!isValid()))
        {
            return false;
        }
    }
    if (LON_UNLIKELY(m_family != addr->getFamily()))
    {
        LON_ERROR(LON_LOG_ROOT) << "bind failed: family mismatch"
                                << " socket=" << m_sockfd << " family=" << m_family
                                << " addr.family = " << addr->getFamily();
        return false;
    }

    if (::bind(m_sockfd, addr->getAddr(), addr->getAddrLen()) == -1)
    {
        LON_ERROR(LON_LOG_ROOT) << "bind failed: bind error"
                                << "socket=" << m_sockfd << " errno = " << errno
                                << " errstr=" << strerror(errno);
        return false;
    }
    getLocalAddress();

    return true;
}

bool Socket::connect(const Address::Ptr &addr, int64_t timeout)
{
    if (!isValid())
    {
        newSocket();
        if (LON_UNLIKELY(!isValid()))
        {
            return false;
        }
    }
    if (LON_UNLIKELY(m_family != addr->getFamily()))
    {
        LON_ERROR(LON_LOG_ROOT) << "connect failed: family mismatch"
                                << " socket=" << m_sockfd << " family=" << m_family
                                << " addr.family = " << addr->getFamily();
        return false;
    }
    if (timeout < 0)
    {
        if (::connect(m_sockfd, addr->getAddr(), addr->getAddrLen()) == -1)
        {
            LON_ERROR(LON_LOG_ROOT) << "connect failed: connect error"
                                    << "socket=" << m_sockfd << " addr=" << addr->toString()
                                    << " errno = " << errno << " errstr=" << strerror(errno);
            close();
            return false;
        }
    }
    else
    {
        if (connect_with_timeout(m_sockfd, addr->getAddr(), addr->getAddrLen(), timeout) == -1)
        {
            LON_ERROR(LON_LOG_ROOT)
                << "connect failed: connect_with_timeout error"
                << "socket=" << m_sockfd << " addr=" << addr->toString() << " timeout=" << timeout
                << " errno = " << errno << " errstr=" << strerror(errno);
            close();
            return false;
        }
    }
    m_is_connected = true;
    getLocalAddress();
    getPeerAddress();

    return true;
}

bool Socket::listen(int backlog)
{
    if (!isValid())
    {
        LON_ERROR(LON_LOG_ROOT) << "listen failed: socket is not valid";
        return false;
    }

    int ret = ::listen(m_sockfd, backlog);
    if (ret)
    {
        LON_ERROR(LON_LOG_ROOT) << "listen failed: listen error"
                                << "socket=" << m_sockfd << " backlog = " << backlog
                                << " errno = " << errno << " errstr=" << strerror(errno);
        return false;
    }
    return true;
}

bool Socket::close()
{
    if (!m_is_connected && m_sockfd == -1)
    {
        return true;
    }
    m_is_connected = false;
    if (m_sockfd != -1)
    {
        ::close(m_sockfd);
        m_sockfd = -1;
    }
    return true;
}

ssize_t Socket::send(const void *buf, size_t len, int flags)
{
    if (!isConnected())
    {
        LON_ERROR(LON_LOG_ROOT) << "send failed: socket is not connected";
        return -1;
    }
    return ::send(m_sockfd, buf, len, flags);
}

ssize_t Socket::send(const iovec *bufs, size_t len, int flags)
{
    if (!isConnected())
    {
        LON_ERROR(LON_LOG_ROOT) << "send failed: socket is not connected";
        return -1;
    }
    msghdr msg;
    memset(&msg, 0, sizeof(msghdr));
    msg.msg_iov    = (iovec *)bufs;
    msg.msg_iovlen = len;

    return ::sendmsg(m_sockfd, &msg, flags);
}

ssize_t Socket::sendto(const void *buf, size_t len, const Address::Ptr &dst, int flags)
{
    if (!isConnected())
    {
        LON_ERROR(LON_LOG_ROOT) << "sendto failed: socket is not connected";
        return -1;
    }
    return ::sendto(m_sockfd, buf, len, flags, dst->getAddr(), dst->getAddrLen());
}

ssize_t Socket::sendto(const iovec *bufs, size_t len, const Address::Ptr &dst, int flags)
{
    if (!isConnected())
    {
        LON_ERROR(LON_LOG_ROOT) << "sendto failed: socket is not connected";
        return -1;
    }
    msghdr msg;
    memset(&msg, 0, sizeof(msghdr));
    msg.msg_iov     = (iovec *)bufs;
    msg.msg_iovlen  = len;
    msg.msg_name    = dst->getAddr();
    msg.msg_namelen = dst->getAddrLen();

    return ::sendmsg(m_sockfd, &msg, flags);
}

ssize_t Socket::recv(void *buf, size_t len, int flags)
{
    if (!isConnected())
    {
        LON_ERROR(LON_LOG_ROOT) << "recv failed: socket is not connected";
        return -1;
    }
    return ::recv(m_sockfd, buf, len, flags);
}

ssize_t Socket::recv(const iovec *bufs, size_t len, int flags)
{
    if (!isConnected())
    {
        LON_ERROR(LON_LOG_ROOT) << "recv failed: socket is not connected";
        return -1;
    }
    msghdr msg;
    memset(&msg, 0, sizeof(msghdr));
    msg.msg_iov    = (iovec *)bufs;
    msg.msg_iovlen = len;

    return ::recvmsg(m_sockfd, &msg, flags);
}

ssize_t Socket::recvfrom(void *buf, size_t len, Address::Ptr &src, int flags)
{
    if (!isConnected())
    {
        LON_ERROR(LON_LOG_ROOT) << "recvfrom failed: socket is not connected";
        return -1;
    }
    auto src_len = src->getAddrLen();
    return ::recvfrom(m_sockfd, buf, len, flags, src->getAddr(), &src_len);
}

ssize_t Socket::recvfrom(const iovec *bufs, size_t len, Address::Ptr &src, int flags)
{
    if (!isConnected())
    {
        LON_ERROR(LON_LOG_ROOT) << "recvfrom failed: socket is not connected";
        return -1;
    }
    msghdr msg;
    memset(&msg, 0, sizeof(msghdr));
    msg.msg_iov     = (iovec *)bufs;
    msg.msg_iovlen  = len;
    msg.msg_name    = src->getAddr();
    msg.msg_namelen = src->getAddrLen();
    return ::recvmsg(m_sockfd, &msg, flags);
}

Address::Ptr Socket::getPeerAddress()
{
    if (m_peer_addr != nullptr)
    {
        return m_peer_addr;
    }
    Address::Ptr addr = nullptr;
    switch (m_family)
    {
    case AF_INET:
        addr = std::make_shared<IPv4Address>();
        break;
    case AF_INET6:
        addr = std::make_shared<IPv6Address>();
        break;
    case AF_UNIX:
        addr = std::make_shared<UnixAddress>();
        break;
    default:
        addr = std::make_shared<UnknownAddress>(m_family);
        break;
    }
    socklen_t len = addr->getAddrLen();
    if (getpeername(m_sockfd, addr->getAddr(), &len) == -1)
    {
        LON_ERROR(LON_LOG_ROOT) << "getPeerAddress failed: getpeername error";
        return std::make_shared<UnknownAddress>(m_family);
    }
    if (m_family == AF_UNIX)
    {
        auto tmp = std::dynamic_pointer_cast<UnixAddress>(addr);
        tmp->setAddrlen(len);
    }
    m_peer_addr = addr;
    return addr;
}

Address::Ptr Socket::getLocalAddress()
{
    if (m_local_addr != nullptr)
    {
        return m_local_addr;
    }
    Address::Ptr addr = nullptr;
    switch (m_family)
    {
    case AF_INET:
        addr = std::make_shared<IPv4Address>();
        break;
    case AF_INET6:
        addr = std::make_shared<IPv6Address>();
        break;
    case AF_UNIX:
        addr = std::make_shared<UnixAddress>();
        break;
    default:
        addr = std::make_shared<UnknownAddress>(m_family);
        break;
    }
    socklen_t len = addr->getAddrLen();
    if (getsockname(m_sockfd, addr->getAddr(), &len) == -1)
    {
        LON_ERROR(LON_LOG_ROOT) << "getLocalAddress failed: getsockname error";
        return std::make_shared<UnknownAddress>(m_family);
    }
    if (m_family == AF_UNIX)
    {
        auto tmp = std::dynamic_pointer_cast<UnixAddress>(addr);
        tmp->setAddrlen(len);
    }
    m_local_addr = addr;
    return addr;
}

int Socket::getFamily() const { return m_family; }

int Socket::getType() const { return m_type; }

int Socket::getProtocol() const { return m_protocol; }

bool Socket::isConnected() const { return m_is_connected; }

bool Socket::isValid() const { return m_sockfd != -1; }

int Socket::getError()
{
    int error  = 0;
    size_t len = sizeof(error);
    if (!getOption(SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len))
    {
        return -1;
    }
    return error;
}

std::ostream &Socket::dump(std::ostream &os) const
{
    os << "Socket( sockfd=" << m_sockfd << ", isconnected=" << m_is_connected
       << ", family=" << m_family << ", type=" << m_type << ", protocol=" << m_protocol;
    if (m_local_addr)
    {
        os << ", localaddr=" << m_local_addr->toString();
    }
    if (m_peer_addr)
    {
        os << ", peeraddr=" << m_peer_addr->toString();
    }
    os << " )";
    return os;
}

int Socket::getSocket() const { return m_sockfd; }

bool Socket::cancelRead()
{
    return scheduler::IOScheduler::getThis()->cancelEvent(m_sockfd, scheduler::IOScheduler::READ);
}

bool Socket::cancelWrite()
{
    return scheduler::IOScheduler::getThis()->cancelEvent(m_sockfd, scheduler::IOScheduler::WRITE);
}

bool Socket::cancelAccept()
{
    return scheduler::IOScheduler::getThis()->cancelEvent(m_sockfd, scheduler::IOScheduler::READ);
}

bool Socket::cancelAll() { return scheduler::IOScheduler::getThis()->cancelAll(m_sockfd); }

void Socket::initSocket()
{
    int val = 1;
    setOption(SOL_SOCKET, SO_REUSEADDR, &val);
    if (m_type == SOCK_STREAM)
    {
        setOption(IPPROTO_TCP, TCP_NODELAY, &val);
    }
}

void Socket::newSocket()
{
    m_sockfd = socket(m_family, m_type, m_protocol);
    if (LON_LIKELY(m_sockfd != -1))
    {
        initSocket();
    }
    else
    {
        LON_ERROR(LON_LOG_ROOT) << "newSocket socket(" << m_family << ", " << m_type << ", "
                                << m_protocol << ") failed"
                                << " errno=" << errno << " errstr=" << strerror(errno);
    }
}

} // namespace net
} // namespace lon