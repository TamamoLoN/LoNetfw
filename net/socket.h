#pragma once
#include "hook/hook.h"
#include "net/address.h"
#ifdef _WIN32
#else
#include <netinet/tcp.h>
#include <sys/socket.h>
#endif

namespace lon
{
namespace net
{
class Socket : public std::enable_shared_from_this<Socket>, util::Nonecopyable
{
  public:
    enum Type
    {
        TCP = SOCK_STREAM,
        UDP = SOCK_DGRAM,
    };
    enum Family
    {
        IPV4 = AF_INET,
        IPV6 = AF_INET6,
#ifndef _WIN32
        UNIX = AF_UNIX,
#endif
    };

  public:
    using Ptr  = std::shared_ptr<Socket>;
    using WPtr = std::weak_ptr<Socket>;
    Socket(int family, int type, int protocol = 0);
    virtual ~Socket();

    static Socket::Ptr create(Socket::Family family = Socket::IPV4,
                              Socket::Type type     = Socket::TCP);
    static Socket::Ptr create(const Address::Ptr &addr, Socket::Type type = Socket::TCP);

    int64_t getSendTimeout() const;
    void setSendTimeout(int64_t timeout);

    int64_t getRecvTimeout() const;
    void setRecvTimeout(int64_t timeout);

    bool getOption(int level, int optname, void *optval, socklen_t *optlen);
    template <typename T> bool getOption(int level, int optname, T &optval)
    {
        socklen_t len = sizeof(optval);
        return getOption(level, optname, &optval, &len);
    }

    bool setOption(int level, int optname, const void *optval, socklen_t optlen);
    template <typename T> bool setOption(int level, int optname, const T &optval)
    {
        return setOption(level, optname, &optval, sizeof(optval));
    }

    Socket::Ptr accept();
    bool bind(const Address::Ptr &addr);
    bool connect(const Address::Ptr &addr, int64_t timeout = -1);
    bool listen(int backlog = SOMAXCONN);
    bool close();

    ssize_t send(const void *buf, size_t len, int flags = 0);
    ssize_t send(const iovec *bufs, size_t len, int flags = 0);
    ssize_t sendto(const void *buf, size_t len, const Address::Ptr &dst, int flags = 0);
    ssize_t sendto(const iovec *bufs, size_t len, const Address::Ptr &dst, int flags = 0);

    ssize_t recv(void *buf, size_t len, int flags = 0);
    ssize_t recv(const iovec *bufs, size_t len, int flags = 0);
    ssize_t recvfrom(void *buf, size_t len, const Address::Ptr &src, int flags = 0);
    ssize_t recvfrom(const iovec *bufs, size_t len, const Address::Ptr &src, int flags = 0);

    Address::Ptr getPeerAddress();
    Address::Ptr getLocalAddress();

    int getFamily() const;
    int getType() const;
    int getProtocol() const;

    bool isConnected() const;
    bool isValid() const;
    int getError();

    std::ostream &dump(std::ostream &os) const;
    std::string toString() const;
    int getSocket() const;

    bool cancelRead();
    bool cancelWrite();
    bool cancelAccept();
    bool cancelAll();

  private:
    bool init(int socketfd);
    void initSocket();
    void newSocket();

  private:
    int m_sockfd;
    int m_family;
    int m_type;
    int m_protocol;
    bool m_is_connected;

    Address::Ptr m_local_addr;
    Address::Ptr m_peer_addr;
};
} // namespace net
} // namespace lon