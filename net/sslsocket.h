#pragma once

#include "net/socket.h"
#include "openssl/ssl.h"

namespace lon
{
namespace net
{
struct SSLInitter
{
    SSLInitter();
    SSLInitter(const SSLInitter &) = delete;
    SSLInitter &operator=(const SSLInitter &) = delete;
    SSLInitter(SSLInitter &&)                 = delete;
    SSLInitter &operator=(SSLInitter &&) = delete;
    static SSLInitter &Instance();
};

class SSLSocket : public Socket
{
  public:
    using Ptr = std::shared_ptr<SSLSocket>;
    SSLSocket(int family, int type, int protocol = 0);
    virtual ~SSLSocket();

    static SSLSocket::Ptr create(Socket::Family family = Socket::IPV4,
                                 Socket::Type type     = Socket::TCP);
    static SSLSocket::Ptr create(const Address::Ptr &addr, Socket::Type type = Socket::TCP);

    bool loadCertificates(const std::string &cert_file, const std::string &key_file);

    virtual Socket::Ptr accept();
    virtual bool bind(const Address::Ptr &addr) override;
    virtual bool connect(const Address::Ptr &addr, int64_t timeout = -1) override;
    virtual bool listen(int backlog = SOMAXCONN) override;
    virtual bool close() override;

    virtual ssize_t send(const void *buf, size_t len, int flags = 0) override;
    virtual ssize_t send(const iovec *bufs, size_t len, int flags = 0) override;
    virtual ssize_t sendto(const void *buf, size_t len, const Address::Ptr &dst,
                           int flags = 0) override;
    virtual ssize_t sendto(const iovec *bufs, size_t len, const Address::Ptr &dst,
                           int flags = 0) override;

    virtual ssize_t recv(void *buf, size_t len, int flags = 0) override;
    virtual ssize_t recv(const iovec *bufs, size_t len, int flags = 0) override;
    virtual ssize_t recvfrom(void *buf, size_t len, const Address::Ptr &src,
                             int flags = 0) override;
    virtual ssize_t recvfrom(const iovec *bufs, size_t len, const Address::Ptr &src,
                             int flags = 0) override;

    virtual std::ostream &dump(std::ostream &os) const override;

  private:
    virtual bool init(int socketfd) override;

  private:
    std::shared_ptr<SSL_CTX> m_ssl_ctx;
    std::shared_ptr<SSL> m_ssl;
};
} // namespace net
} // namespace lon