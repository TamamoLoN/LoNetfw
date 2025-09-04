#include "lonetfw/lonetfw.h"

using namespace lon::net;

void test_address()
{
    IPv4Address ipv4(INADDR_LOOPBACK, 8080);
    LON_INFO(LON_LOG_ROOT) << "ipv4 addr:" << ipv4.toString();
    LON_INFO(LON_LOG_ROOT) << "ipv4 broadcast:" << ipv4.broadcastAddress(24)->toString();
    LON_INFO(LON_LOG_ROOT) << "ipv4 network:" << ipv4.networkAddress(24)->toString();
    LON_INFO(LON_LOG_ROOT) << "ipv4 netmask:" << ipv4.subnetMask(24)->toString();

    uint8_t buf[16] = {0};
    buf[15]         = 1;
    // memcpy(buf, "::1", 3);
    IPv6Address ipv6(buf, 8080);
    LON_INFO(LON_LOG_ROOT) << "ipv6 addr:" << ipv6.toString();

    LON_WARN(LON_LOG_ROOT) << "===parse===";
    std::vector<Address::Ptr> addrs = {};
    auto ret                        = Address::parse(addrs, "www.baidu.com", AF_UNSPEC, 0, 0);
    if (!ret)
    {
        LON_ERROR(LON_LOG_ROOT) << "parse error";
    }
    else
    {
        for (const auto &addr : addrs)
        {
            LON_INFO(LON_LOG_ROOT) << addr->toString();
        }
    }
}

void test_interface()
{
    LON_WARN(LON_LOG_ROOT) << "===test_interface===";
    std::multimap<std::string, std::pair<Address::Ptr, uint32_t>> addrs;

    bool ret = Address::getInterfaceAddresses(addrs, AF_UNSPEC);
    if (!ret)
    {
        LON_ERROR(LON_LOG_ROOT) << "parse error";
    }
    else
    {
        for (const auto &it : addrs)
        {
            LON_INFO(LON_LOG_ROOT)
                << it.first << "-" << it.second.first->toString() << ":" << it.second.second;
        }
    }
}

void test_ipv4()
{
    LON_WARN(LON_LOG_ROOT) << "===test_ipv4===";
    auto test = IPAddress::create("sdsad1", 1);
    auto ipv4 = IPAddress::create("www.baidu.com", 8080);
    LON_INFO(LON_LOG_ROOT) << "ipv4 addr:" << ipv4->toString();
    LON_INFO(LON_LOG_ROOT) << "ipv4 broadcast:" << ipv4->broadcastAddress(24)->toString();
    LON_INFO(LON_LOG_ROOT) << "ipv4 network:" << ipv4->networkAddress(24)->toString();
    LON_INFO(LON_LOG_ROOT) << "ipv4 netmask:" << ipv4->subnetMask(24)->toString();
}

void test_ipv6()
{
    LON_WARN(LON_LOG_ROOT) << "===test_ipv6===";
    auto ipv6 = IPAddress::create("2408:871a:2100:186c::ff:b07e:3fbc", 8080);
    LON_INFO(LON_LOG_ROOT) << "ipv6 addr:" << ipv6->toString();
    LON_INFO(LON_LOG_ROOT) << "ipv6 broadcast:" << ipv6->broadcastAddress(24)->toString();
    LON_INFO(LON_LOG_ROOT) << "ipv6 network:" << ipv6->networkAddress(24)->toString();
    LON_INFO(LON_LOG_ROOT) << "ipv6 netmask:" << ipv6->subnetMask(24)->toString();
}

void test_socket()
{
    LON_WARN(LON_LOG_ROOT) << "===test_socket===";
    auto ios = std::make_shared<lon::scheduler::IOScheduler>(
        1, true, "io_scheduler", lon::config::ConfigInitter::Instance().config_fiber->getData());
    ios->schedule([]() {
        // usleep(1);
        IPAddress::Ptr addr = nullptr;
        Address::parseIPAddress(addr, "ifconfig.me:80");
        LON_INFO(LON_LOG_ROOT) << "addr:" << addr->toString();

        auto sockfd = Socket::create(addr);
        if (!sockfd->connect(addr))
        {
            LON_ERROR(LON_LOG_ROOT) << "connect error";
        }
        else
        {
            LON_INFO(LON_LOG_ROOT) << "connect ok";
        }
        sockfd->setRecvTimeout(
            lon::config::ConfigInitter::Instance().config_tcp_timeout->getData());
        std::string buf =
            "GET / HTTP/1.1\r\nHost: ifconfig.me\r\nUser-Agent: curl/7.68.0\r\nAccept: */*\r\n\r\n";
        int ret = sockfd->send(buf.data(), buf.size());
        if (ret <= 0)
        {
            LON_ERROR(LON_LOG_ROOT) << "send error";
            return;
        }
        buf.clear();
        buf.resize(4096);
        ret = sockfd->recv(&buf[0], buf.size());
        if (ret <= 0)
        {
            LON_ERROR(LON_LOG_ROOT) << "recv error, ret = " << ret;
            return;
        }
        buf.resize(ret);
        LON_INFO(LON_LOG_ROOT) << "recv:" << buf;
    });
}

void test_bytearray()
{
    srand(time(nullptr));
    ByteArray ba;
    LON_INFO(LON_LOG_ROOT) << "dev endian: "
                           << ((LON_ENDIAN == LON_LITTLE_ENDIAN) ? "little" : "big");
    LON_INFO(LON_LOG_ROOT) << "ba endian: " << (ba.isLittleEndian() ? "little" : "big");
#define XX(type, len, readfun, writefun, node_size)                                                \
    {                                                                                              \
        ByteArray ba(node_size);                                                                   \
        std::vector<type> datas;                                                                   \
        for (int i = 0; i < len; ++i)                                                              \
        {                                                                                          \
            datas.push_back(rand());                                                               \
        }                                                                                          \
        for (const auto &it : datas)                                                               \
        {                                                                                          \
            ba.writefun(it);                                                                       \
        }                                                                                          \
        ba.setPosition(0);                                                                         \
        for (int i = 0; i < datas.size(); ++i)                                                     \
        {                                                                                          \
            auto data = ba.readfun();                                                              \
            std::stringstream ss;                                                                  \
            ss << i << " - " << (int)data << " - " << (int)datas[i];                               \
            LON_ASSERT_(data == datas[i], ss.str());                                               \
            /*LON_INFO(LON_LOG_ROOT) << i << " - " << (int)data << " - " << (int)datas[i]; */      \
        }                                                                                          \
        LON_ASSERT(ba.getReadSize() == 0);                                                         \
        LON_INFO(LON_LOG_ROOT) << #readfun "/" #writefun << "(" #type ") len=" << len              \
                               << ", node_size=" << node_size << ", count=" << ba.count()          \
                               << ", size=" << ba.size();                                          \
    }
    XX(int8_t, 100, readFInt8, writeFInt8, 100)
    XX(uint8_t, 100, readFUInt8, writeFUInt8, 100)
    XX(int16_t, 100, readFInt16, writeFInt16, 100)
    XX(uint16_t, 100, readFUInt16, writeFUInt16, 100)
    XX(int32_t, 100, readFInt32, writeFInt32, 100)
    XX(uint32_t, 100, readFUInt32, writeFUInt32, 100)
    XX(int64_t, 100, readFInt64, writeFInt64, 100)
    XX(uint64_t, 100, readFUInt64, writeFUInt64, 100)

    XX(int32_t, 100, readInt32, writeInt32, 100)
    XX(uint32_t, 100, readUInt32, writeUInt32, 100)
    XX(int64_t, 100, readInt64, writeInt64, 100)
    XX(uint64_t, 100, readUInt64, writeUInt64, 100)
#undef XX

#define XX(type, len, readfun, writefun, node_size, path)                                          \
    {                                                                                              \
        ByteArray ba(node_size);                                                                   \
        std::vector<type> datas;                                                                   \
        for (int i = 0; i < len; ++i)                                                              \
        {                                                                                          \
            datas.push_back(rand());                                                               \
        }                                                                                          \
        for (const auto &it : datas)                                                               \
        {                                                                                          \
            ba.writefun(it);                                                                       \
        }                                                                                          \
        ba.setPosition(0);                                                                         \
        for (int i = 0; i < datas.size(); ++i)                                                     \
        {                                                                                          \
            auto data = ba.readfun();                                                              \
            std::stringstream ss;                                                                  \
            ss << i << " - " << (int)data << " - " << (int)datas[i];                               \
            LON_ASSERT_(data == datas[i], ss.str());                                               \
            /*LON_INFO(LON_LOG_ROOT) << i << " - " << (int)data << " - " << (int)datas[i]; */      \
        }                                                                                          \
        LON_ASSERT(ba.getReadSize() == 0);                                                         \
        LON_INFO(LON_LOG_ROOT) << #readfun "/" #writefun << "(" #type ") len=" << len              \
                               << ", node_size=" << node_size << ", count=" << ba.count()          \
                               << ", size=" << ba.size();                                          \
        ba.setPosition(0);                                                                         \
        ba.writeToFile(path "/" #readfun "-" #writefun "-" #type ".data");                         \
        ByteArray ba1(node_size * 2);                                                              \
        ba1.readFromFile(path "/" #readfun "-" #writefun "-" #type ".data");                       \
        ba1.setPosition(0);                                                                        \
        LON_ASSERT(ba1.toString() == ba.toString());                                               \
        LON_ASSERT(ba.getPosition() == 0);                                                         \
        LON_ASSERT(ba1.getPosition() == 0);                                                        \
    }
    XX(int8_t, 100, readFInt8, writeFInt8, 100, "./.tmp")
    XX(uint8_t, 100, readFUInt8, writeFUInt8, 100, "./.tmp")
    XX(int16_t, 100, readFInt16, writeFInt16, 100, "./.tmp")
    XX(uint16_t, 100, readFUInt16, writeFUInt16, 100, "./.tmp")
    XX(int32_t, 100, readFInt32, writeFInt32, 100, "./.tmp")
    XX(uint32_t, 100, readFUInt32, writeFUInt32, 100, "./.tmp")
    XX(int64_t, 100, readFInt64, writeFInt64, 100, "./.tmp")
    XX(uint64_t, 100, readFUInt64, writeFUInt64, 100, "./.tmp")

    XX(int32_t, 100, readInt32, writeInt32, 100, "./.tmp")
    XX(uint32_t, 100, readUInt32, writeUInt32, 100, "./.tmp")
    XX(int64_t, 100, readInt64, writeInt64, 100, "./.tmp")
    XX(uint64_t, 100, readUInt64, writeUInt64, 100, "./.tmp")
#undef XX
}

int main(int argc, char const *argv[])
{
    lon::config::Config::parseFromYaml(".config/log.yaml");
    // test_address();
    // test_interface();
    // test_ipv4();
    // test_ipv6();
    // test_socket();
    test_bytearray();

    return 0;
}
