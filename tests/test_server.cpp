#include "lonetfw/lonetfw.h"

void test_tcp_server()
{
    lon::net::Address::Ptr addr;
    // auto addr_unix = std::make_shared<lon::net::UnixAddress>("/tmp/unix_addr");
    lon::net::Address::parse(addr, "0.0.0.0:8080", AF_INET);

    LON_INFO(LON_LOG_ROOT) << "addr=" << addr->toString();
    // LON_INFO(LON_LOG_ROOT) << "addr_unix=" << addr_unix->toString();

    std::vector<lon::net::Address::Ptr> addrs;
    addrs.push_back(addr);
    // addrs.push_back(addr_unix);

    auto server = std::make_shared<lon::server::TcpServer>(
        lon::scheduler::IOScheduler::getThis(), lon::scheduler::IOScheduler::getThis(),
        lon::config::GlobalConfig::Instance().config_tcp_server_client_timeout->getData(),
        "test_server");
    std::vector<lon::net::Address::Ptr> bind_failed_addrs;
    while (!server->bind(addrs, bind_failed_addrs))
    {
        bind_failed_addrs.clear();
        sleep(2);
    }
    server->start();
}

class EchoServer : public lon::server::TcpServer
{
  public:
    EchoServer(int type)
        : TcpServer(
              lon::scheduler::IOScheduler::getThis(), lon::scheduler::IOScheduler::getThis(),
              lon::config::GlobalConfig::Instance().config_tcp_server_client_timeout->getData(),
              "echo_server"),
          m_type(type)
    {
    }

    void handleClient(const lon::net::Socket::Ptr &client) override
    {
        LON_INFO(LON_LOG_ROOT) << "handleClient: " << client->toString();
        lon::util::ByteArray::Ptr buffer = std::make_shared<lon::util::ByteArray>();
        while (true)
        {
            buffer->clear();
            std::vector<iovec> iovecs;
            buffer->getWriteBuffers(iovecs, 1024);

            auto ret = client->recv(&iovecs[0], iovecs.size());
            if (ret == 0)
            {
                LON_INFO(LON_LOG_ROOT) << "client closed: " << client->toString();
                break;
            }
            else if (ret < 0)
            {
                LON_ERROR(LON_LOG_ROOT) << "recv error: " << client->toString() << " error=" << ret
                                        << " errno=" << errno << " errstr=" << strerror(errno);
                break;
            }
            buffer->setPosition(buffer->getPosition() + ret);
            buffer->setPosition(0);
            if (m_type == 1) // 文本
            {
                LON_INFO(LON_LOG_ROOT) << "recv text: " << buffer->toString();
            }
            else
            {
                LON_INFO(LON_LOG_ROOT) << "recv hex: " << buffer->toStringHex();
            }
        }
    }

  private:
    int m_type; // 1:文本，2:十六进制
};

void test_echo_server()
{
    lon::net::Address::Ptr addr;
    lon::net::Address::parse(addr, "0.0.0.0:8080", AF_INET);

    LON_INFO(LON_LOG_ROOT) << "addr=" << addr->toString();

    std::vector<lon::net::Address::Ptr> addrs;
    addrs.push_back(addr);

    auto server = std::make_shared<EchoServer>(1);
    std::vector<lon::net::Address::Ptr> bind_failed_addrs;
    while (!server->bind(addrs, bind_failed_addrs))
    {
        bind_failed_addrs.clear();
        sleep(2);
    }
    server->start();
}

int main(int argc, char const *argv[])
{
    lon::config::Config::parseFromYaml(".config/log.yaml");
    auto ios = std::make_shared<lon::scheduler::IOScheduler>(
        2, true, "io_scheduler", lon::config::GlobalConfig::Instance().config_fiber->getData());
    // ios->schedule(test_tcp_server);
    ios->schedule(test_echo_server);

    return 0;
}