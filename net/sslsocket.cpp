#include "net/sslsocket.h"

namespace lon
{
namespace net
{
static auto g_logger       = LON_LOG_ROOT;
static auto &g_ssl_initter = SSLInitter::Instance();

SSLInitter::SSLInitter()
{
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
}

SSLInitter &SSLInitter::Instance()
{
    static SSLInitter instance;
    return instance;
}

SSLSocket::SSLSocket(int family, int type, int protocol) : Socket(family, type, protocol)
{
    // TODO: 实现构造函数
}

SSLSocket::~SSLSocket() {}

SSLSocket::Ptr SSLSocket::create(Socket::Family family, Socket::Type type)
{
    SSLSocket::Ptr socket = nullptr;
    switch (family)
    {
    case IPV4:
        switch (type)
        {
        case TCP:
            socket = std::make_shared<SSLSocket>(IPV4, TCP, 0);
            break;
        case UDP:
            // UDP没有连接和绑定端口，所以在这里直接创建socket
            socket = std::make_shared<SSLSocket>(IPV4, UDP, 0);
            socket->newSocket();
            socket->m_is_connected = true;
            break;
        default:
            break;
        }
    case IPV6:
        switch (type)
        {
        case TCP:
            socket = std::make_shared<SSLSocket>(IPV6, TCP, 0);
            break;
        case UDP:
            socket = std::make_shared<SSLSocket>(IPV6, UDP, 0);
            socket->newSocket();
            socket->m_is_connected = true;
            break;
        default:
            break;
        }
#ifndef _WIN32
    case UNIX:
        switch (type)
        {
        case TCP:
            socket = std::make_shared<SSLSocket>(UNIX, TCP, 0);
            break;
        case UDP:
            socket = std::make_shared<SSLSocket>(UNIX, UDP, 0);
            break;
        default:
            break;
        }
#endif
    default:
        break;
    }
    return socket;
}

SSLSocket::Ptr SSLSocket::create(const Address::Ptr &addr, Socket::Type type)
{
    SSLSocket::Ptr socket = nullptr;
    switch (type)
    {
    case TCP:
        socket = std::make_shared<SSLSocket>(addr->getFamily(), TCP, 0);
        break;
    case UDP:
        socket = std::make_shared<SSLSocket>(addr->getFamily(), UDP, 0);
        socket->newSocket();
        socket->m_is_connected = true;
        break;
    default:
        break;
    }
    return socket;
}

bool SSLSocket::loadCertificates(const std::string &cert_file, const std::string &key_file)
{
    m_ssl_ctx.reset(SSL_CTX_new(SSLv23_server_method()), SSL_CTX_free);
    if (SSL_CTX_use_certificate_chain_file(m_ssl_ctx.get(), cert_file.c_str()) != 1)
    {
        LON_ERROR(g_logger) << "SSL_CTX_use_certificate_chain_file(" << cert_file << ") error";
        return false;
    }
    if (SSL_CTX_use_PrivateKey_file(m_ssl_ctx.get(), key_file.c_str(), SSL_FILETYPE_PEM) != 1)
    {
        LON_ERROR(g_logger) << "SSL_CTX_use_PrivateKey_file(" << key_file << ") error";
        return false;
    }
    if (SSL_CTX_check_private_key(m_ssl_ctx.get()) != 1)
    {
        LON_ERROR(g_logger) << "SSL_CTX_check_private_key cert_file=" << cert_file
                            << " key_file=" << key_file;
        return false;
    }
    return true;
}

Socket::Ptr SSLSocket::accept()
{
    SSLSocket::Ptr socket = std::make_shared<SSLSocket>(m_family, m_type, m_protocol);
    int new_socket        = ::accept(m_sockfd, nullptr, nullptr);
    if (new_socket == -1)
    {
        LON_ERROR(g_logger) << "accept failed: "
                            << "socket=" << m_sockfd << " new_socket = " << new_socket
                            << " errno = " << errno << " errstr=" << strerror(errno);
        return nullptr;
    }
    socket->m_ssl_ctx = m_ssl_ctx;
    if (socket->init(new_socket))
    {
        return socket;
    }
    return nullptr;
}

bool SSLSocket::init(int socketfd)
{
    bool res = Socket::init(socketfd);
    if (res)
    {
        m_ssl.reset(SSL_new(m_ssl_ctx.get()), SSL_free);
        SSL_set_fd(m_ssl.get(), m_sockfd);
        res = (SSL_accept(m_ssl.get()) == 1);
    }
    return res;
}

bool SSLSocket::bind(const Address::Ptr &addr) { return Socket::bind(addr); }

bool SSLSocket::connect(const Address::Ptr &addr, int64_t timeout)
{
    bool res = Socket::connect(addr, timeout);
    if (res)
    {
        m_ssl_ctx.reset(SSL_CTX_new(SSLv23_client_method()), SSL_CTX_free);
        m_ssl.reset(SSL_new(m_ssl_ctx.get()), SSL_free);
        SSL_set_fd(m_ssl.get(), m_sockfd);
        res = (SSL_connect(m_ssl.get()) == 1);
    }
    return res;
}

bool SSLSocket::listen(int backlog) { return Socket::listen(backlog); }

bool SSLSocket::close() { return Socket::close(); }

ssize_t SSLSocket::send(const void *buf, size_t len, int flags)
{
    if (!m_ssl)
    {
        return -1;
    }
    return SSL_write(m_ssl.get(), buf, len);
}

ssize_t SSLSocket::send(const iovec *bufs, size_t len, int flags)
{
    if (!m_ssl)
    {
        return -1;
    }
    int total = 0;
    for (size_t i = 0; i < len; ++i)
    {
        int res = SSL_write(m_ssl.get(), bufs[i].iov_base, bufs[i].iov_len);
        if (res <= 0)
        {
            return res;
        }
        total += res;
        if (res != (int)bufs[i].iov_len)
        {
            break;
        }
    }
    return total;
}

ssize_t SSLSocket::sendto(const void *buf, size_t len, const Address::Ptr &dst, int flags)
{
    LON_ASSERT(false);
    return -1;
}

ssize_t SSLSocket::sendto(const iovec *bufs, size_t len, const Address::Ptr &dst, int flags)
{
    LON_ASSERT(false);
    return -1;
}

ssize_t SSLSocket::recv(void *buf, size_t len, int flags)
{
    if (!m_ssl)
    {
        return -1;
    }
    return SSL_read(m_ssl.get(), buf, len);
}

ssize_t SSLSocket::recv(const iovec *bufs, size_t len, int flags)
{
    if (!m_ssl)
    {
        return -1;
    }
    int total = 0;
    for (size_t i = 0; i < len; ++i)
    {
        int res = SSL_read(m_ssl.get(), bufs[i].iov_base, bufs[i].iov_len);
        if (res <= 0)
        {
            return res;
        }
        total += res;
        if (res != (int)bufs[i].iov_len)
        {
            break;
        }
    }
    return total;
}

ssize_t SSLSocket::recvfrom(void *buf, size_t len, const Address::Ptr &src, int flags)
{
    LON_ASSERT(false);
    return -1;
}

ssize_t SSLSocket::recvfrom(const iovec *bufs, size_t len, const Address::Ptr &src, int flags)
{
    LON_ASSERT(false);
    return -1;
}

std::ostream &SSLSocket::dump(std::ostream &os) const
{
    os << "SSLSocket( sockfd=" << m_sockfd << ", isconnected=" << m_is_connected
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
} // namespace net
} // namespace lon