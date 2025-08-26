#include "socket/address.h"

namespace lon
{
namespace socket
{
int Address::getFamily() const { return getAddr()->sa_family; }

std::string Address::toString() const
{
    std::stringstream ss;
    insert(ss);
    return ss.str();
}

bool Address::operator<(const Address &val) const
{
    socklen_t minlen = std::min(getAddrLen(), val.getAddrLen());
    int ret          = memcmp(getAddr(), val.getAddr(), minlen);
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
    if (m_addr_len <= sizeof(m_addr.sun_path))
    {
        throw std::runtime_error("path too long");
    }
    memcpy(m_addr.sun_path, path.c_str(), m_addr_len);
    m_addr_len += offsetof(sockaddr_un, sun_path);
}

sockaddr *UnixAddress::getAddr() const { return (sockaddr *)&m_addr; }

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

IPv4Address::IPv4Address(uint32_t address, uint32_t port)
{
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin_family      = AF_INET;
    m_addr.sin_addr.s_addr = util::byteswapOnLittleEndian(address);
    setPort(port);
}

sockaddr *IPv4Address::getAddr() const { return (sockaddr *)&m_addr; }

socklen_t IPv4Address::getAddrLen() const { return sizeof(m_addr); }

std::ostream &IPv4Address::insert(std::ostream &os) const
{
    uint32_t addr = util::byteswapOnLittleEndian(m_addr.sin_addr.s_addr);
    uint32_t port = util::byteswapOnLittleEndian(m_addr.sin_port);
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
}

IPAddress::Ptr IPv4Address::networkAddress(uint32_t prefix_len) {}

IPAddress::Ptr IPv4Address::subnetMask(uint32_t prefix_len) {}

uint32_t IPv4Address::getPort() const { return util::byteswapOnLittleEndian(m_addr.sin_port); }

void IPv4Address::setPort(uint32_t port) { m_addr.sin_port = util::byteswapOnLittleEndian(port); }

IPv6Address::IPv6Address()
{
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin6_family = AF_INET6;
}

IPv6Address::IPv6Address(const char *address, uint32_t port)
{
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin6_family = AF_INET6;
    memcpy(m_addr.sin6_addr.__in6_u.__u6_addr8, address, 16);
    setPort(port);
}

sockaddr *IPv6Address::getAddr() const { return (sockaddr *)&m_addr; }

socklen_t IPv6Address::getAddrLen() const { return sizeof(m_addr); }

std::ostream &IPv6Address::insert(std::ostream &os) const
{
    os << "[";
    uint16_t *addr = (uint16_t *)m_addr.sin6_addr.__in6_u.__u6_addr8;
    uint32_t port  = util::byteswapOnLittleEndian(m_addr.sin6_port);
    bool use_zeros = false;
    for (ssize_t i = 0; i < 8; i++)
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

IPAddress::Ptr IPv6Address::broadcastAddress(uint32_t prefix_len) {}

IPAddress::Ptr IPv6Address::networkAddress(uint32_t prefix_len) {}

IPAddress::Ptr IPv6Address::subnetMask(uint32_t prefix_len) {}

uint32_t IPv6Address::getPort() const { return util::byteswapOnLittleEndian(m_addr.sin6_port); }

void IPv6Address::setPort(uint32_t port) { m_addr.sin6_port = util::byteswapOnLittleEndian(port); }

} // namespace socket
} // namespace lon