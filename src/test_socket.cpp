#include "config/config.h"
#include "log/logger.h"
#include "net/socket.h"

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
        Address::parseIPAddress(addr, "www.baidu.com:80");
        LON_INFO(LON_LOG_ROOT) << "addr:" << addr->toString();

        auto sockfd = Socket::create(addr);
        if (!sockfd->connect(addr, 1000))
        {
            LON_ERROR(LON_LOG_ROOT) << "connect error";
        }
        else
        {
            LON_INFO(LON_LOG_ROOT) << "connect ok";
        }
        std::string buf = "GET / HTTP/1.1\r\n\r\n";
        int ret         = sockfd->send(buf.data(), buf.size());
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
            LON_ERROR(LON_LOG_ROOT) << "recv error";
            return;
        }
        buf.resize(ret);
        LON_INFO(LON_LOG_ROOT) << "recv:" << buf;
    });
}

int main(int argc, char const *argv[])
{
    lon::config::Config::parseFromYaml(".config/log.yaml");
    // test_address();
    // test_interface();
    // test_ipv4();
    // test_ipv6();
    test_socket();
    return 0;
}
