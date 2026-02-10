#pragma once

#include "scheduler/schedulermanager.h"
#include "server/serverfactory.h"
#include "server/tcpserver.h"
#include "system/daemon.h"
#include "system/env.h"

namespace lon
{
namespace system
{
class Application
{
  public:
    explicit Application();
    virtual ~Application();
    Application(Application const &) = delete;
    Application &operator=(Application const &) = delete;
    Application(Application &&)                 = delete;
    Application &operator=(Application &&) = delete;

    static Application &Instance();

    bool init(int argc, char **argv);
    bool run();

    bool getServer(const std::string &type, std::vector<server::TcpServer::Ptr> &servers);

  protected:
  private:
    int main(int argc, char **argv);
    int runTask();

  private:
    int m_argc;
    char **m_argv;
    std::unordered_map<std::string, std::vector<server::TcpServer::Ptr>> m_servers;
    scheduler::IOScheduler::Ptr m_main_ioscheduler;
};
} // namespace system
} // namespace lon