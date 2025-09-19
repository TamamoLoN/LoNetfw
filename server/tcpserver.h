#pragma once

#include "net/socket.h"
#include "scheduler/ioscheduler.h"

namespace lon
{
namespace server
{
// 防止在handleClient之前，TcpServer就被析构
class TcpServer : public std::enable_shared_from_this<TcpServer>, util::Nonecopyable
{
  public:
    using Ptr = std::shared_ptr<TcpServer>;
    TcpServer(scheduler::IOScheduler *scheduler        = scheduler::IOScheduler::getThis(),
              scheduler::IOScheduler *accept_scheduler = scheduler::IOScheduler::getThis(),
              size_t client_timeout = 1000 * 60 * 2, const std::string &name = "UNKNOWN");
    virtual ~TcpServer();

    // bind: bind + listen
    virtual bool bind(const net::Address::Ptr &addr);
    virtual bool bind(const std::vector<net::Address::Ptr> &addrs,
                      std::vector<net::Address::Ptr> &bind_failed_addrs);
    virtual bool start();
    virtual bool stop();

    void setClientTimeout(size_t client_timeout);
    size_t getClientTimeout() const;
    void setName(const std::string &name);
    std::string getName() const;
    bool isStop() const;
    void setStop(bool is_stop);

  protected:
    virtual void handleClient(const net::Socket::Ptr &client);
    virtual void startAccept(const net::Socket::Ptr &socket);

  private:
    scheduler::IOScheduler *m_scheduler;        //线程池: IO调度器
    scheduler::IOScheduler *m_accept_scheduler; //线程池: 接受连接调度器
    std::vector<net::Socket::Ptr> m_sockets;
    size_t m_client_timeout; // 若客户端在一定时间内没有发送数据，则断开连接(ms)
    std::string m_name;
    bool m_is_stop;
};
} // namespace server
} // namespace lon