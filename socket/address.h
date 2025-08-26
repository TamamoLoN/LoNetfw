#pragma once

#include "util/util.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

namespace lon
{
namespace socket
{
class Address
{
  public:
    using Ptr          = std::shared_ptr<Address>;
    Address()          = default;
    virtual ~Address() = default;

    int getFamily() const;
    virtual sockaddr *getAddr() const                    = 0;
    virtual socklen_t getAddrLen() const                 = 0;
    virtual std::ostream &insert(std::ostream &os) const = 0;
    std::string toString() const;

    bool operator<(const Address &val) const;
    bool operator==(const Address &val) const;
    bool operator!=(const Address &val) const;
};

class UnixAddress : public Address
{
  public:
    using Ptr = std::shared_ptr<UnixAddress>;
    UnixAddress();
    UnixAddress(const std::string &path);
    virtual ~UnixAddress() = default;

    sockaddr *getAddr() const override;
    socklen_t getAddrLen() const override;
    std::ostream &insert(std::ostream &os) const override;

  private:
    struct sockaddr_un m_addr;
    socklen_t m_addr_len;
};

class UnknownAddress : public Address
{
  public:
    using Ptr = std::shared_ptr<UnknownAddress>;
    UnknownAddress(int family);
    virtual ~UnknownAddress() = default;

    sockaddr *getAddr() const override;
    socklen_t getAddrLen() const override;
    std::ostream &insert(std::ostream &os) const override;

  private:
    sockaddr m_addr;
};

class IPAddress : public Address
{
  public:
    using Ptr            = std::shared_ptr<IPAddress>;
    IPAddress()          = default;
    virtual ~IPAddress() = default;

    virtual IPAddress::Ptr broadcastAddress(uint32_t prefix_len) = 0;
    virtual IPAddress::Ptr networkAddress(uint32_t prefix_len)   = 0;
    virtual IPAddress::Ptr subnetMask(uint32_t prefix_len)       = 0;

    virtual uint32_t getPort() const    = 0;
    virtual void setPort(uint32_t port) = 0;
};

class IPv4Address : public IPAddress
{
  public:
    using Ptr = std::shared_ptr<IPv4Address>;
    IPv4Address(uint32_t address = INADDR_ANY, uint32_t port = 0);
    virtual ~IPv4Address() = default;

    sockaddr *getAddr() const override;
    socklen_t getAddrLen() const override;
    std::ostream &insert(std::ostream &os) const override;

    IPAddress::Ptr broadcastAddress(uint32_t prefix_len) override;
    IPAddress::Ptr networkAddress(uint32_t prefix_len) override;
    IPAddress::Ptr subnetMask(uint32_t prefix_len) override;

    uint32_t getPort() const override;
    void setPort(uint32_t port) override;

  private:
    sockaddr_in m_addr;
};

class IPv6Address : public IPAddress
{
  public:
    using Ptr = std::shared_ptr<IPv6Address>;
    IPv6Address();
    IPv6Address(const char *address, uint32_t port = 0);
    virtual ~IPv6Address() = default;

    sockaddr *getAddr() const override;
    socklen_t getAddrLen() const override;
    std::ostream &insert(std::ostream &os) const override;

    IPAddress::Ptr broadcastAddress(uint32_t prefix_len) override;
    IPAddress::Ptr networkAddress(uint32_t prefix_len) override;
    IPAddress::Ptr subnetMask(uint32_t prefix_len) override;

    uint32_t getPort() const override;
    void setPort(uint32_t port) override;

  private:
    sockaddr_in6 m_addr;
};

} // namespace socket
} // namespace lon