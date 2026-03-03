#include "net/address.h"
#ifdef _WIN32
#include <iphlpapi.h>
#endif

namespace lon
{
namespace net
{
static auto g_logger = LON_LOG_ROOT;
Address::Ptr Address::create(const sockaddr *addr, socklen_t addr_len)
{
    if (addr == nullptr)
    {
        return nullptr;
    }
    Address::Ptr result = nullptr;
    switch (addr->sa_family)
    {
    case AF_INET:
        result = std::make_shared<IPv4Address>(*(const sockaddr_in *)addr);
        break;
    case AF_INET6:
        result = std::make_shared<IPv6Address>(*(const sockaddr_in6 *)addr);
        break;
    default:
        result = std::make_shared<UnknownAddress>(*addr);
        break;
    }
    return result;
}

bool Address::parse(std::vector<Address::Ptr> &addrs, const std::string &host, int family, int type,
                    int protocol)
{
    addrinfo addr;
    addrinfo *result = nullptr;
    addrinfo *next   = nullptr;
    memset(&addr, 0, sizeof(addr));
    addr.ai_flags     = 0;
    addr.ai_family    = family;
    addr.ai_socktype  = type;
    addr.ai_protocol  = protocol;
    addr.ai_addrlen   = 0;
    addr.ai_canonname = nullptr;
    addr.ai_addr      = nullptr;
    addr.ai_next      = nullptr;

    std::string node    = "";
    const char *service = nullptr;
    // 检查ipv6地址: 类似[fe80::f816:3eff:fece:e49b]
    if (!host.empty() && host[0] == '[')
    {
        const char *endipv6 = (const char *)memchr(host.c_str() + 1, ']', host.size() - 1);
        if (endipv6)
        {
            if (*(endipv6 + 1) == ':')
            {
                service = endipv6 + 2;
            }
            node = host.substr(0, endipv6 - host.c_str());
        }
    }
    // 检查 node service
    if (node.empty())
    {
        service = (const char *)memchr(host.c_str(), ':', host.size());
        if (service)
        {
            if (!memchr(service + 1, ':', host.c_str() + host.size() - service - 1))
            {
                node = host.substr(0, service - host.c_str());
                ++service;
            }
        }
    }

    if (node.empty())
    {
        node = host;
    }
    int ret = getaddrinfo(node.c_str(), service, &addr, &result);
    if (ret)
    {
        LON_ERROR(g_logger) << "Address::find(addrs, " << host << ", " << family << ", " << type
                            << ", " << protocol << ") getaddrinfo error, ret=" << ret
                            << " errno=" << errno << " errstr=" << strerror(errno);
        return false;
    }

    next = result;
    while (next)
    {
        addrs.push_back(create(next->ai_addr, (socklen_t)next->ai_addrlen));
        next = next->ai_next;
    }
    freeaddrinfo(result);

    return !addrs.empty();
}

bool Address::parse(Address::Ptr &addr, const std::string &host, int family, int type, int protocol)
{
    std::vector<Address::Ptr> addrs = {};
    if (parse(addrs, host, family, type, protocol))
    {
        addr = addrs.at(0);
        return true;
    }
    return !addrs.empty();
}

bool Address::parseIPAddress(std::shared_ptr<IPAddress> &addr, const std::string &host, int family,
                             int type, int protocol)
{
    std::vector<Address::Ptr> addrs = {};
    if (parse(addrs, host, family, type, protocol))
    {
        for (const auto &it : addrs)
        {
            auto ret = std::dynamic_pointer_cast<IPAddress>(it);
            if (ret)
            {
                addr = ret;
                return true;
            }
        }
    }
    return !addrs.empty();
}

bool Address::getInterfaceAddresses(
    std::multimap<std::string, std::pair<Address::Ptr, uint32_t>> &addrs, int family)
{
#ifdef _WIN32
    ULONG flags  = GAA_FLAG_INCLUDE_PREFIX;
    ULONG bufLen = 15 * 1024;
    std::vector<char> buffer(bufLen);

    PIP_ADAPTER_ADDRESSES adapters = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buffer.data());

    ULONG ret = GetAdaptersAddresses(family == AF_UNSPEC ? AF_UNSPEC : family, flags, nullptr,
                                     adapters, &bufLen);

    if (ret == ERROR_BUFFER_OVERFLOW)
    {
        buffer.resize(bufLen);
        adapters = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buffer.data());
        ret      = GetAdaptersAddresses(family == AF_UNSPEC ? AF_UNSPEC : family, flags, nullptr,
                                   adapters, &bufLen);
    }

    if (ret != NO_ERROR)
    {
        LON_ERROR(g_logger) << "GetAdaptersAddresses failed, ret=" << ret;
        return false;
    }

    for (auto ad = adapters; ad; ad = ad->Next)
    {
        std::string ifname = ad->AdapterName;

        for (auto ua = ad->FirstUnicastAddress; ua; ua = ua->Next)
        {
            int sa_family = ua->Address.lpSockaddr->sa_family;
            if (family != AF_UNSPEC && family != sa_family)
            {
                continue;
            }

            Address::Ptr addr   = nullptr;
            uint32_t prefix_len = ua->OnLinkPrefixLength;

            if (sa_family == AF_INET)
            {
                addr = create(ua->Address.lpSockaddr, sizeof(sockaddr_in));
            }
            else if (sa_family == AF_INET6)
            {
                addr = create(ua->Address.lpSockaddr, sizeof(sockaddr_in6));
            }

            if (addr)
            {
                addrs.emplace(ifname, std::make_pair(addr, prefix_len));
            }
        }
    }

    return true;
#else
    struct ifaddrs *next = nullptr;
    struct ifaddrs *addr = nullptr;
    int ret              = getifaddrs(&addr);
    if (ret)
    {
        LON_ERROR(g_logger) << "Address::getInterfaceAddresse(addrs, " << family
                            << ") getifaddrs error, ret=" << ret << " errno=" << errno
                            << " errstr=" << strerror(errno);
        return false;
    }
    next = addr;
    try
    {
        while (next)
        {
            uint32_t prefix_len = ~0u;
            Address::Ptr a      = nullptr;
            if (family != AF_UNSPEC && family != next->ifa_addr->sa_family)
            {
                next = next->ifa_next;
                continue;
            }
            switch (next->ifa_addr->sa_family)
            {
            case AF_INET:
            {
                a                = create(next->ifa_addr, sizeof(sockaddr_in));
                uint32_t netmask = ((sockaddr_in *)next->ifa_netmask)->sin_addr.s_addr;
                prefix_len       = countBytes(netmask);
            }
            break;
            case AF_INET6:
            {
                a                 = create(next->ifa_addr, sizeof(sockaddr_in6));
                in6_addr &netmask = ((sockaddr_in6 *)next->ifa_netmask)->sin6_addr;
                prefix_len        = 0;
                for (int i = 0; i < 16; ++i)
                {
                    prefix_len += countBytes(netmask.__in6_u.__u6_addr8[i]);
                }
            }
            break;
            default:
                break;
            }
            if (a)
            {
                addrs.insert(std::make_pair(next->ifa_name, std::make_pair(a, prefix_len)));
            }
            next = next->ifa_next;
        }
    }
    catch (...)
    {
        LON_ERROR(g_logger) << "Address::getInterfaceAddresse(addrs, " << family << ") error";
        freeifaddrs(addr);
        return false;
    }
    freeifaddrs(addr);
#endif
    return !addrs.empty();
}

bool Address::getInterfaceAddresses(std::vector<std::pair<Address::Ptr, uint32_t>> &addrs,
                                    const std::string &_interface, int family)
{
    if (_interface.empty() || _interface == "*")
    {
        if (family == AF_UNSPEC || family == AF_INET)
        {
            addrs.push_back(std::make_pair(std::make_shared<IPv4Address>(), 0u));
        }
        if (family == AF_UNSPEC || family == AF_INET6)
        {
            addrs.push_back(std::make_pair(std::make_shared<IPv6Address>(), 0u));
        }
        return true;
    }
    std::multimap<std::string, std::pair<Address::Ptr, uint32_t>> results;
    if (!getInterfaceAddresses(results, family))
    {
        return false;
    }
    auto its = results.equal_range(_interface);
    for (; its.first != its.second; ++its.first)
    {
        addrs.push_back(its.first->second);
    }
    return !addrs.empty();
}

int Address::getFamily() const { return getAddr()->sa_family; }

std::string Address::toString() const
{
    std::stringstream ss;
    insert(ss);
    return ss.str();
}

bool Address::operator<(const Address &val) const
{
#ifdef _WIN32
    socklen_t minlen = min(getAddrLen(), val.getAddrLen());
#else
    socklen_t minlen = std::min(getAddrLen(), val.getAddrLen());
#endif
    int ret = memcmp(getAddr(), val.getAddr(), minlen);
    if (ret < 0)
    {
        return true;
    }
    else if (ret > 0)
    {
        return false;
    }
    else if (getAddrLen() < val.getAddrLen())
    {
        return true;
    }
    return false;
}

bool Address::operator==(const Address &val) const
{
    return getAddrLen() == val.getAddrLen() && memcmp(getAddr(), val.getAddr(), getAddrLen()) == 0;
}

bool Address::operator!=(const Address &val) const { return !(*this == val); }

IPAddress::Ptr IPAddress::create(const std::string &address, const uint16_t &port)
{
    addrinfo addr;
    addrinfo *result      = nullptr;
    IPAddress::Ptr ipaddr = nullptr;
    memset(&addr, 0, sizeof(addr));

    addr.ai_flags  = AI_CANONNAME;
    addr.ai_family = AF_UNSPEC;
    int ret        = getaddrinfo(address.c_str(), NULL, &addr, &result);
    if (ret)
    {
        LON_ERROR(g_logger) << "IPAddress::create(" << address << ", " << port
                            << ") error, ret=" << ret << " errno=" << errno
                            << " errstr=" << strerror(errno);
        freeaddrinfo(result);
        return nullptr;
    }
    try
    {
        ipaddr = std::dynamic_pointer_cast<IPAddress>(
            Address::create(result->ai_addr, (socklen_t)result->ai_addrlen));
        if (ipaddr)
        {
            ipaddr->setPort(port);
        }
        freeaddrinfo(result);
        return ipaddr;
    }
    catch (...)
    {
        freeaddrinfo(result);
        return nullptr;
    }
    freeaddrinfo(result);
    return nullptr;
}

#ifndef _WIN32
UnixAddress::UnixAddress()
{
    memset(&m_addr, 0, sizeof(m_addr));
    const size_t MAX_PATH_LEN = sizeof(((sockaddr_un *)0)->sun_path) - 1;
    m_addr.sun_family         = AF_UNIX;
    m_addr_len                = offsetof(sockaddr_un, sun_path) + MAX_PATH_LEN;
}

UnixAddress::UnixAddress(const std::string &path)
{
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sun_family = AF_UNIX;
    m_addr_len        = path.size() + 1;
    if (!path.empty() && path[0] == '\0')
    {
        --m_addr_len;
    }
    if (m_addr_len > sizeof(m_addr.sun_path))
    {
        throw std::runtime_error("path too long");
    }
    memcpy(m_addr.sun_path, path.c_str(), m_addr_len);
    m_addr_len += offsetof(sockaddr_un, sun_path);
}

sockaddr *UnixAddress::getAddr() const { return (sockaddr *)&m_addr; }

void UnixAddress::setAddrlen(socklen_t len) { m_addr_len = len; }

socklen_t UnixAddress::getAddrLen() const { return m_addr_len; }

std::ostream &UnixAddress::insert(std::ostream &os) const
{
    if (m_addr_len > offsetof(sockaddr_un, sun_path) && m_addr.sun_path[0] == '\0')
    {
        return os << "\\0"
                  << std::string(m_addr.sun_path + 1,
                                 m_addr_len - offsetof(sockaddr_un, sun_path) - 1);
    }
    return os << m_addr.sun_path;
}

std::string UnixAddress::getPath() const
{
    std::stringstream ss;
    if (m_addr_len > offsetof(sockaddr_un, sun_path) && m_addr.sun_path[0] == '\0')
    {
        ss << "\\0"
           << std::string(m_addr.sun_path + 1, m_addr_len - offsetof(sockaddr_un, sun_path) - 1);
    }
    else
    {
        ss << m_addr.sun_path;
    }
    return ss.str();
}
#endif

UnknownAddress::UnknownAddress(sockaddr addr) { m_addr = addr; }

UnknownAddress::UnknownAddress(int family)
{
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sa_family = family;
}

sockaddr *UnknownAddress::getAddr() const { return (sockaddr *)&m_addr; }

socklen_t UnknownAddress::getAddrLen() const { return sizeof(m_addr); }

std::ostream &UnknownAddress::insert(std::ostream &os) const
{
    return os << "[UnknownAddress family = " << m_addr.sa_family << "]";
}

IPv4Address::IPv4Address(sockaddr_in address) { m_addr = address; }

IPv4Address::IPv4Address(uint32_t address, uint16_t port)
{
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin_family      = AF_INET;
    m_addr.sin_addr.s_addr = util::byteswapOnLittleEndian(address);
    setPort(port);
}

IPv4Address::Ptr IPv4Address::create(const std::string &address, const uint16_t &port)
{
    auto addr = std::make_shared<IPv4Address>();
    addr->setPort(port);
    int ret = inet_pton(AF_INET, address.c_str(), &addr->m_addr.sin_addr.s_addr);
    if (ret <= 0)
    {
        LON_ERROR(g_logger) << "IPv4Address::create(" << address << ", " << port
                            << ") error ,ret=" << ret << " errno=" << errno
                            << " errstr=" << strerror(errno);
        return nullptr;
    }
    return addr;
}

sockaddr *IPv4Address::getAddr() const { return (sockaddr *)&m_addr; }

socklen_t IPv4Address::getAddrLen() const { return sizeof(m_addr); }

std::ostream &IPv4Address::insert(std::ostream &os) const
{
    uint32_t addr = util::byteswapOnLittleEndian(m_addr.sin_addr.s_addr);
    uint16_t port = util::byteswapOnLittleEndian(m_addr.sin_port);
    os << ((addr >> 24) & 0xff) << "." << ((addr >> 16) & 0xff) << "." << ((addr >> 8) & 0xff)
       << "." << (addr & 0xff);
    os << ":" << port;
    return os;
}

IPAddress::Ptr IPv4Address::broadcastAddress(uint32_t prefix_len)
{
    if (prefix_len > 32)
    {
        return nullptr;
    }
    sockaddr_in b_addr(m_addr);
    b_addr.sin_addr.s_addr |= util::byteswapOnLittleEndian(createMask<uint32_t>(prefix_len));
    return std::make_shared<IPv4Address>(b_addr);
}

IPAddress::Ptr IPv4Address::networkAddress(uint32_t prefix_len)
{
    if (prefix_len > 32)
    {
        return nullptr;
    }
    sockaddr_in n_addr(m_addr);
    n_addr.sin_addr.s_addr &= util::byteswapOnLittleEndian(createMask<uint32_t>(prefix_len));
    return std::make_shared<IPv4Address>(n_addr);
}

IPAddress::Ptr IPv4Address::subnetMask(uint32_t prefix_len)
{
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = ~util::byteswapOnLittleEndian(createMask<uint32_t>(prefix_len));
    addr.sin_port        = m_addr.sin_port;

    return std::make_shared<IPv4Address>(addr);
}

uint16_t IPv4Address::getPort() const { return util::byteswapOnLittleEndian(m_addr.sin_port); }

void IPv4Address::setPort(uint16_t port) { m_addr.sin_port = util::byteswapOnLittleEndian(port); }

IPv6Address::IPv6Address()
{
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin6_family = AF_INET6;
}

IPv6Address::IPv6Address(sockaddr_in6 address) { m_addr = address; }

IPv6Address::IPv6Address(const uint8_t address[16], uint16_t port)
{
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin6_family = AF_INET6;
#ifdef _WIN32
    memcpy(m_addr.sin6_addr.u.Byte, address, 16);
#else
    memcpy(m_addr.sin6_addr.__in6_u.__u6_addr8, address, 16);
#endif
    setPort(port);
}

IPv6Address::Ptr IPv6Address::create(const std::string &address, const uint16_t &port)
{
    auto addr = std::make_shared<IPv6Address>();
    addr->setPort(port);
    int ret = inet_pton(AF_INET6, address.c_str(), &addr->m_addr.sin6_addr);
    if (ret <= 0)
    {
        LON_ERROR(g_logger) << "IPv6Address::create(" << address << ", " << port
                            << ") error ,ret=" << ret << " errno=" << errno
                            << " errstr=" << strerror(errno);
        return nullptr;
    }
    return addr;
}

sockaddr *IPv6Address::getAddr() const { return (sockaddr *)&m_addr; }

socklen_t IPv6Address::getAddrLen() const { return sizeof(m_addr); }

std::ostream &IPv6Address::insert(std::ostream &os) const
{
    os << "[";
#ifdef _WIN32
    uint16_t *addr = (uint16_t *)m_addr.sin6_addr.u.Byte;
#else
    uint16_t *addr = (uint16_t *)m_addr.sin6_addr.__in6_u.__u6_addr8;
#endif
    uint16_t port  = util::byteswapOnLittleEndian(m_addr.sin6_port);
    bool use_zeros = false;
    for (ssize_t i = 0; i < 8; ++i)
    {
        if (addr[i] == 0 && !use_zeros)
        {
            continue;
        }
        if (i && addr[i - 1] == 0 && !use_zeros)
        {
            os << ":";
            use_zeros = true;
        }
        if (i)
        {
            os << ":";
        }
        os << std::hex << (int)util::byteswapOnLittleEndian(addr[i]) << std::dec;
    }
    if (!use_zeros && addr[7] == 0)
    {
        os << "::";
    }
    os << "]:" << port;
    return os;
}

IPAddress::Ptr IPv6Address::broadcastAddress(uint32_t prefix_len)
{
    sockaddr_in6 b_addr(m_addr);
#ifdef _WIN32
    b_addr.sin6_addr.u.Byte[prefix_len / 8] |= createMask<uint8_t>(prefix_len % 8);
    for (int i = prefix_len / 8 + 1; i < 16; ++i)
    {
        b_addr.sin6_addr.u.Byte[i] = 0xff;
    }
#else
    b_addr.sin6_addr.__in6_u.__u6_addr8[prefix_len / 8] |= createMask<uint8_t>(prefix_len % 8);
    for (int i = prefix_len / 8 + 1; i < 16; ++i)
    {
        b_addr.sin6_addr.__in6_u.__u6_addr8[i] = 0xff;
    }
#endif
    return std::make_shared<IPv6Address>(b_addr);
}

IPAddress::Ptr IPv6Address::networkAddress(uint32_t prefix_len)
{
    sockaddr_in6 n_addr(m_addr);
#ifdef _WIN32
    n_addr.sin6_addr.u.Byte[prefix_len / 8] &= createMask<uint8_t>(prefix_len % 8);
#else
    n_addr.sin6_addr.__in6_u.__u6_addr8[prefix_len / 8] &= createMask<uint8_t>(prefix_len % 8);
#endif

    return std::make_shared<IPv6Address>(n_addr);
}

IPAddress::Ptr IPv6Address::subnetMask(uint32_t prefix_len)
{
    sockaddr_in6 addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
#ifdef _WIN32
    addr.sin6_addr.u.Byte[prefix_len / 8] = ~createMask<uint8_t>(prefix_len % 8);
#else
    addr.sin6_addr.__in6_u.__u6_addr8[prefix_len / 8] = ~createMask<uint8_t>(prefix_len % 8);
#endif
    addr.sin6_port = m_addr.sin6_port;
    for (int i = 0; i < prefix_len / 8; ++i)
    {
#ifdef _WIN32
        addr.sin6_addr.u.Byte[i] = 0xff;
#else
        addr.sin6_addr.__in6_u.__u6_addr8[i] = 0xff;
#endif
    }
    return std::make_shared<IPv6Address>(addr);
}

uint16_t IPv6Address::getPort() const { return util::byteswapOnLittleEndian(m_addr.sin6_port); }

void IPv6Address::setPort(uint16_t port) { m_addr.sin6_port = util::byteswapOnLittleEndian(port); }

} // namespace net
} // namespace lon