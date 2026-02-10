#pragma once

#include "server/tcpserver.h"

namespace lon
{
namespace server
{
class ServerFactory
{
  public:
    ServerFactory(const ServerFactory &) = delete;
    ServerFactory &operator=(const ServerFactory &) = delete;
    ServerFactory(ServerFactory &&)                 = delete;
    ServerFactory &operator=(ServerFactory &&) = delete;
    static ServerFactory &Instance();
    void registerServer(
        const std::string &name,
        const std::function<TcpServer::Ptr(scheduler::IOScheduler *scheduler,
                                           scheduler::IOScheduler *accept_scheduler,
                                           const config::ConfigServer &config_server)> &factory);
    TcpServer::Ptr create(const std::string &name, scheduler::IOScheduler *scheduler,
                          scheduler::IOScheduler *accept_scheduler,
                          const config::ConfigServer &config_server);

  private:
    ServerFactory();
    std::unordered_map<std::string,
                       std::function<TcpServer::Ptr(scheduler::IOScheduler *scheduler,
                                                    scheduler::IOScheduler *accept_scheduler,
                                                    const config::ConfigServer &config_server)>>
        m_server_factory;
};
} // namespace server
} // namespace lon
