#include "server/serverfactory.h"

namespace lon
{
namespace server
{
ServerFactory::ServerFactory() : m_server_factory({}) {}

ServerFactory &ServerFactory::Instance()
{
    static ServerFactory instance;
    return instance;
}

void ServerFactory::registerServer(
    const std::string &name,
    const std::function<TcpServer::Ptr(scheduler::IOScheduler *scheduler,
                                       scheduler::IOScheduler *accept_scheduler,
                                       const config::ConfigServer &config_server)> &factory)
{
    m_server_factory[name] = std::move(factory);
}

TcpServer::Ptr ServerFactory::create(const std::string &name, scheduler::IOScheduler *scheduler,
                                     scheduler::IOScheduler *accept_scheduler,
                                     const config::ConfigServer &config_server)
{
    auto it = m_server_factory.find(name);
    if (it == m_server_factory.end())
    {
        return nullptr;
    }
    return it->second(scheduler, accept_scheduler, config_server);
}

} // namespace server
} // namespace lon