#include "server/tcpserver.h"

namespace lon
{
namespace server
{
TcpServer::TcpServer(scheduler::IOScheduler *scheduler, scheduler::IOScheduler *accept_scheduler,
                     size_t client_timeout, const std::string &name)
    : m_scheduler(scheduler), m_accept_scheduler(accept_scheduler),
      m_client_timeout(client_timeout), m_is_stop(true), m_sockets({}),
      m_name(name + "/LoNetfw-" LONETFW_VERSION)
{
}

TcpServer::~TcpServer()
{
    for (const auto &socket : m_sockets)
    {
        socket->close();
    }
    m_sockets.clear();
}

bool TcpServer::bind(const net::Address::Ptr &addr)
{
    std::vector<net::Address::Ptr> addrs = {addr};
    std::vector<net::Address::Ptr> bind_failed_addrs;
    return bind(addrs, bind_failed_addrs);
}

bool TcpServer::bind(const std::vector<net::Address::Ptr> &addrs,
                     std::vector<net::Address::Ptr> &bind_failed_addrs)
{
    for (const auto &addr : addrs)
    {
        auto socket = net::Socket::create(addr, net::Socket::Type::TCP);
        if (!socket->bind(addr))
        {
            LON_ERROR(LON_LOG_ROOT) << "[" << getName() << "] bind failed: " << addr->toString();
            bind_failed_addrs.push_back(addr);
            continue;
        }
        if (!socket->listen())
        {
            LON_ERROR(LON_LOG_ROOT) << "[" << getName() << "] listen failed: " << addr->toString();
            bind_failed_addrs.push_back(addr);
            continue;
        }
        m_sockets.push_back(socket);
    }
    if (!bind_failed_addrs.empty())
    {
        m_sockets.clear();
        return false;
    }
    for (const auto &socket : m_sockets)
    {
        LON_INFO(LON_LOG_ROOT) << "[" << getName() << "] bind success: " << socket->toString();
    }
    return true;
}

bool TcpServer::start()
{
    if (!isStop())
    {
        return true;
    }
    setStop(false);
    for (const auto &socket : m_sockets)
    {
        m_accept_scheduler->schedule(
            std::bind(&TcpServer::startAccept, shared_from_this(), socket));
    }
    return true;
}

bool TcpServer::stop()
{
    if (isStop())
    {
        return true;
    }
    setStop(true);
    // 防止调度器还在运行时，TcpServer被析构
    auto self = shared_from_this();
    m_accept_scheduler->schedule([this, self]() {
        for (const auto &socket : m_sockets)
        {
            socket->cancelAll();
            socket->close();
        }
        m_sockets.clear();
    });

    return true;
}

void TcpServer::setClientTimeout(size_t client_timeout) { m_client_timeout = client_timeout; }

size_t TcpServer::getClientTimeout() const { return m_client_timeout; }

void TcpServer::setName(const std::string &name) { m_name = name; }

std::string TcpServer::getName() const { return m_name; }

bool TcpServer::isStop() const { return m_is_stop; }

void TcpServer::setStop(bool is_stop) { m_is_stop = is_stop; }

void TcpServer::handleClient(const net::Socket::Ptr &client)
{
    LON_INFO(LON_LOG_ROOT) << "[" << getName() << "] handleClient: " << client->toString();
}

void TcpServer::startAccept(const net::Socket::Ptr &socket)
{
    while (!isStop())
    {
        auto client = socket->accept();
        if (!client)
        {
            LON_ERROR(LON_LOG_ROOT)
                << "[" << getName() << "] accept failed: " << socket->toString();
        }
        else
        {
            client->setRecvTimeout(m_client_timeout);
            m_scheduler->schedule(std::bind(&TcpServer::handleClient, shared_from_this(), client));
        }
    }
}

} // namespace server
} // namespace lon