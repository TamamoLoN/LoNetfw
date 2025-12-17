#pragma once

#include "log/logger.h"
#include "util/util.h"
#include <sys/types.h>
#ifdef _WIN32

#else
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/un.h>
#endif

namespace lon
{
namespace net
{
template <typename T> static T createMask(uint32_t bits) { return (1 << sizeof(T) * 8 - bits) - 1; }

// 计算一个数二进制有多少个1
template <typename T> static uint32_t countBytes(T val)
{
    uint32_t result = 0;
    for (; val; ++result)
    {
        val &= val - 1;
    }
    return result;
}

class IPAddress;
class Address
{
  public:
    using Ptr          = std::shared_ptr<Address>;
    Address()          = default;
    virtual ~Address() = default;

    static Address::Ptr create(const sockaddr *addr, socklen_t addr_len);
    // family = AF_UNSPEC为任意类型， 默认为IPV4
    static bool parse(std::vector<Address::Ptr> &addrs, const std::string &host,
                      int family = AF_INET, int type = 0, int protocol = 0);
    static bool parse(Address::Ptr &addr, const std::string &host, int family = AF_INET,
                      int type = 0, int protocol = 0);
    static bool parseIPAddress(std::shared_ptr<IPAddress> &addr, const std::string &host,
                               int family = AF_INET, int type = 0, int protocol = 0);
    // getInterfaceAddresses： uint32_t:子网掩码长度
    static bool
    getInterfaceAddresses(std::multimap<std::string, std::pair<Address::Ptr, uint32_t>> &addrs,
                          int family = AF_INET);
    static bool getInterfaceAddresses(std::vector<std::pair<Address::Ptr, uint32_t>> &addrs,
                                      const std::string &_interface, int family = AF_INET);

    int getFamily() const;
    virtual sockaddr *getAddr() const                    = 0;
    virtual socklen_t getAddrLen() const                 = 0;
    virtual std::ostream &insert(std::ostream &os) const = 0;
    std::string toString() const;

    bool operator<(const Address &val) const;
    bool operator==(const Address &val) const;
    bool operator!=(const Address &val) const;
};

#ifndef _WIN32
class UnixAddress : public Address
{
  public:
    using Ptr = std::shared_ptr<UnixAddress>;
    UnixAddress();
    UnixAddress(const std::string &path);
    virtual ~UnixAddress() = default;

    sockaddr *getAddr() const override;
    void setAddrlen(socklen_t len);
    socklen_t getAddrLen() const override;
    std::ostream &insert(std::ostream &os) const override;

  private:
    struct sockaddr_un m_addr;
    socklen_t m_addr_len;
};
#endif

class UnknownAddress : public Address
{
  public:
    using Ptr = std::shared_ptr<UnknownAddress>;
    UnknownAddress(sockaddr addr);
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

    static IPAddress::Ptr create(const std::string &address, const uint16_t &port);
    virtual IPAddress::Ptr broadcastAddress(uint32_t prefix_len) = 0;
    virtual IPAddress::Ptr networkAddress(uint32_t prefix_len)   = 0;
    virtual IPAddress::Ptr subnetMask(uint32_t prefix_len)       = 0;

    virtual uint16_t getPort() const    = 0;
    virtual void setPort(uint16_t port) = 0;
};

class IPv4Address : public IPAddress
{
  public:
    using Ptr = std::shared_ptr<IPv4Address>;
    IPv4Address(sockaddr_in address);
    IPv4Address(uint32_t address = INADDR_ANY, uint16_t port = 0);
    virtual ~IPv4Address() = default;

    static IPv4Address::Ptr create(const std::string &address, const uint16_t &port);
    sockaddr *getAddr() const override;
    socklen_t getAddrLen() const override;
    std::ostream &insert(std::ostream &os) const override;

    IPAddress::Ptr broadcastAddress(uint32_t prefix_len) override;
    IPAddress::Ptr networkAddress(uint32_t prefix_len) override;
    IPAddress::Ptr subnetMask(uint32_t prefix_len) override;

    uint16_t getPort() const override;
    void setPort(uint16_t port) override;

  private:
    sockaddr_in m_addr;
};

class IPv6Address : public IPAddress
{
  public:
    using Ptr = std::shared_ptr<IPv6Address>;
    IPv6Address();
    IPv6Address(sockaddr_in6 address);
    IPv6Address(const uint8_t address[16], uint16_t port = 0);
    virtual ~IPv6Address() = default;

    static IPv6Address::Ptr create(const std::string &address, const uint16_t &port);
    sockaddr *getAddr() const override;
    socklen_t getAddrLen() const override;
    std::ostream &insert(std::ostream &os) const override;

    IPAddress::Ptr broadcastAddress(uint32_t prefix_len) override;
    IPAddress::Ptr networkAddress(uint32_t prefix_len) override;
    IPAddress::Ptr subnetMask(uint32_t prefix_len) override;

    uint16_t getPort() const override;
    void setPort(uint16_t port) override;

  private:
    sockaddr_in6 m_addr;
};

} // namespace net
} // namespace lon