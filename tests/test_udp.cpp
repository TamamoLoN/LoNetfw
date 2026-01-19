#include "lonetfw/lonetfw.h"

static auto g_logger = LON_LOG_ROOT;

void test_udp_server()
{
    lon::net::IPAddress::Ptr addr = nullptr;
    lon::net::Address::parseIPAddress(addr, "0.0.0.0:8081");
    auto sock = lon::net::Socket::create(addr, lon::net::Socket::UDP);
    if (sock->bind(addr))
    {
        LON_INFO(g_logger) << "udp bind : " << addr->toString();
    }
    else
    {
        LON_ERROR(g_logger) << "udp bind : " << addr->toString() << " fail";
        return;
    }
    while (true)
    {
        char buff[1024];
        lon::net::Address::Ptr from(new lon::net::IPv4Address);
        int len = sock->recvfrom(buff, 1024, from);
        if (len > 0)
        {
            buff[len] = '\0';
            LON_INFO(g_logger) << "recv: " << buff << " from: " << from->toString();
            len = sock->sendto(buff, len, from);
            if (len < 0)
            {
                LON_INFO(g_logger)
                    << "send: " << buff << " to: " << from->toString() << " error=" << len;
            }
        }
    }
}

void test_udp_client(const std::string &ip, int port)
{
    lon::net::IPAddress::Ptr addr = nullptr;
    lon::net::Address::parseIPAddress(addr, ip);
    if (!addr)
    {
        LON_ERROR(g_logger) << "invalid ip: " << ip;
        return;
    }
    addr->setPort(port);

    auto sock = lon::net::Socket::create(addr, lon::net::Socket::UDP);

    lon::scheduler::IOScheduler::getThis()->schedule([addr, sock]() {
        LON_INFO(g_logger) << "begin recv";
        while (true)
        {
            char buff[1024];
            int len = sock->recvfrom(buff, 1024, addr);
            if (len > 0)
            {
                std::cout << std::endl
                          << "recv: " << std::string(buff, len) << " from: " << addr->toString()
                          << std::endl;
            }
        }
    });
    sleep(1);
    while (true)
    {
        std::string line;
        std::cout << "input>";
        std::getline(std::cin, line);
        if (!line.empty())
        {
            int len = sock->sendto(line.c_str(), line.size(), addr);
            if (len < 0)
            {
                int err = sock->getError();
                LON_ERROR(g_logger)
                    << "send error err=" << err << " errstr=" << strerror(err) << " len=" << len
                    << " addr=" << addr->toString() << " sock=" << sock->toString();
            }
            else
            {
                LON_INFO(g_logger) << "send " << line << " len:" << len;
            }
        }
    }
}

int main(int argc, char *argv[])
{
    lon::util::ArgumentParser parser;
    parser.addDescription("test udp client & server");
    parser.addArgument(std::vector<std::string>{"--server", "-s"})
        ->help("以服务端模式启动")
        ->action("store_true");

    parser.addArgument(std::vector<std::string>{"--client", "-c"})
        ->help("以客户端模式启动")
        ->action("store_true");

    parser.addArgument(std::vector<std::string>{"--host", "-H"})
        ->help("客户端模式启动目标地址")
        ->defaultValue("127.0.0.1");

    parser.addArgument(std::vector<std::string>{"--port", "-p"})
        ->help("客户端模式启动目标端口")
        ->defaultValue("8081");
    try
    {
        parser.parse(argc, argv);
        auto server = parser.get<bool>("--server");
        auto client = parser.get<bool>("--client");
        if (server && client)
        {
            parser.handleError("不能同时以服务端和客户端模式启动");
        }
    }
    catch (...)
    {
        return 0;
    }
    auto is_server = parser.get<bool>("--server");
    lon::config::Config::parseFromYaml(".config/log.yaml");
    auto ios = std::make_shared<lon::scheduler::IOScheduler>(1);

    if (is_server)
    {
        ios->schedule(test_udp_server);
    }
    else
    {
        auto host = parser.get<std::string>("--host");
        auto port = parser.get<int>("--port");
        ios->schedule(std::bind(test_udp_client, host, port));
    }

    return 0;
}
