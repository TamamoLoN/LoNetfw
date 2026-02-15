#include "system/application.h"

namespace lon
{
namespace system
{
static auto g_logger = LON_LOG_ROOT;

Application::Application() : m_argc(0), m_argv(nullptr), m_servers({}), m_main_ioscheduler(nullptr)
{
}

Application::~Application() {}

Application &Application::Instance()
{
    static Application instance;
    return instance;
}

bool Application::init(int argc, char **argv)
{
    m_argc = argc;
    m_argv = argv;

    ENVMGR.addArgument(std::vector<std::string>{"--mode", "-m"})
        ->choices({"terminal", "daemon"})
        ->defaultValue("terminal")
        ->help("program run as terminal or daemon mode")
        ->nargs("1");

    ENVMGR.addArgument(std::vector<std::string>{"-c", "--config"})
        ->help("program read config file directory path")
        ->defaultValue("../.config")
        ->nargs("1");

    ENVMGR.addArgument(std::vector<std::string>{"-p", "--plugin"})
        ->help("program read config file directory path")
        ->defaultValue("../.plugin")
        ->nargs("1");

    if (!ENVMGR.init(m_argc, m_argv))
    {
        return false;
    }

    auto mode        = ENVMGR.get<std::string>("--mode");
    auto config_path = ENVMGR.getConfigPath();
    auto plugin_path = ENVMGR.getPluginPath();
    LON_INFO(g_logger) << "mode=" << mode << ", config_path=" << config_path;
    config::Config::parseFromDir(config_path);

    PLUGINMGR.init(plugin_path);
    std::vector<Plugin::Ptr> plugins{};
    PLUGINMGR.getAll(plugins);

    for (const auto &plugin : plugins)
    {
        plugin->onBeforeArgParse(argc, argv);
    }

    for (const auto &plugin : plugins)
    {
        plugin->onAfterArgParse(argc, argv);
    }
    plugins.clear();

    return true;
}

bool Application::run()
{
    auto mode      = ENVMGR.get<std::string>("--mode");
    bool is_daemon = (mode == "daemon");
    return start_daemon(
        m_argc, m_argv,
        std::bind(&Application::main, this, std::placeholders::_1, std::placeholders::_2),
        is_daemon);
}

bool Application::getServer(const std::string &type, std::vector<server::TcpServer::Ptr> &servers)
{
    auto it = m_servers.find(type);
    if (it != m_servers.end())
    {
        servers = it->second;
        return true;
    }
    LON_ERROR(g_logger) << "server type not found: " << type;
    return false;
}

int Application::main(int argc, char **argv)
{
    LON_INFO(g_logger) << "main";
    auto config_path = ENVMGR.getConfigPath();
    config::Config::parseFromDir(config_path);
    {
        auto tmp_dir      = ENVMGR.getCwd() + "/.tmp";
        auto pidfile_path = tmp_dir + "/" + ENVMGR.getProgram() + ".pid";
        if (LON_UNLIKELY(!util::FSUtil::isFileExist(tmp_dir)))
        {
            if (!util::FSUtil::mkdir(tmp_dir))
            {
                LON_ERROR(g_logger) << "mkdir " << tmp_dir << " failed";
                return -1;
            }
        }
        std::ofstream ofs(pidfile_path);
        if (!ofs)
        {
            LON_ERROR(g_logger) << "open pidfile " << pidfile_path << " failed";
            return -1;
        }
        ofs << getpid();
    }

    m_main_ioscheduler.reset(new scheduler::IOScheduler(
        1, true, "io_scheduler", lon::config::GlobalConfig::Instance().config_fiber->getData()));
    m_main_ioscheduler->schedule(std::bind(&Application::runTask, this));
    m_main_ioscheduler->addTimer(
        2000, []() {}, true);
    m_main_ioscheduler->stop();
    return 0;
}

int Application::runTask()
{
    bool onload_failed = false;
    std::vector<Plugin::Ptr> plugins{};
    PLUGINMGR.getAll(plugins);
    for (const auto &plugin : plugins)
    {
        if (!plugin->onLoad())
        {
            LON_ERROR(g_logger) << "plugin onLoad() failed, name=" << plugin->getName()
                                << " version=" << plugin->getVersion()
                                << " filename=" << plugin->getPath();
            onload_failed = true;
        }
    }
    if (onload_failed)
    {
        exit(0);
    }

    // 初始化IO调度器，这里默认只用IOScheduler
    auto config_schedulers = G_CONFIG.config_schedulers->getData();
    for (const auto &schedulers : config_schedulers)
    {
        auto scheduler_name = schedulers.first;
        auto scheduler      = schedulers.second;
        int scheduler_count = util::getOr(scheduler, "scheduler_count", 1);
        int thread_count    = util::getOr(scheduler, "thread_count", 1);
        size_t fiber_stack_size =
            util::getOr(scheduler, "fiber_stack_size", G_CONFIG.config_fiber->getData());
        for (int cnt = 0; cnt < scheduler_count; ++cnt)
        {
            scheduler::IOScheduler::Ptr s = nullptr;
            if (!cnt)
            {
                s = std::make_shared<scheduler::IOScheduler>(thread_count, false, scheduler_name,
                                                             fiber_stack_size);
            }
            else
            {
                s = std::make_shared<scheduler::IOScheduler>(
                    thread_count, false,
                    scheduler_name + "_" + util::lexical_cast<std::string>(cnt), fiber_stack_size);
            }

            SCHEDMGR.addScheduler(s);
        }
    }
    if (!SCHEDMGR.start())
    {
        LON_ERROR(g_logger) << "start scheduler failed";
        exit(0);
    }

    // 初始化服务器
    auto config_servers = G_CONFIG.config_servers->getData();
    for (const auto &config_server : config_servers)
    {
        std::vector<net::Address::Ptr> addrs{};
        for (const auto &config_addr : config_server.addrs)
        {
            size_t pos = config_addr.find(":");
            if (pos == std::string::npos)
            {
                addrs.push_back(net::UnixAddress::Ptr(new net::UnixAddress(config_addr)));
                continue;
            }
            int32_t port = atoi(config_addr.substr(pos + 1).c_str());
            // 127.0.0.1
            auto addr = net::IPAddress::create(config_addr.substr(0, pos).c_str(), port);
            if (addr)
            {
                addrs.push_back(addr);
                continue;
            }
            std::vector<std::pair<net::Address::Ptr, uint32_t>> if_addrs{};
            if (net::Address::getInterfaceAddresses(if_addrs, config_addr.substr(0, pos)))
            {
                for (auto &if_addr : if_addrs)
                {
                    auto ipaddr = std::dynamic_pointer_cast<net::IPAddress>(if_addr.first);
                    if (ipaddr)
                    {
                        ipaddr->setPort(atoi(config_addr.substr(pos + 1).c_str()));
                    }
                    addrs.push_back(ipaddr);
                }
                continue;
            }
            net::Address::Ptr aaddr = nullptr;
            net::Address::parse(aaddr, config_addr);
            if (aaddr)
            {
                addrs.push_back(aaddr);
                continue;
            }
            LON_ERROR(g_logger) << "invalid address: " << config_addr;
            exit(0);
        }
        auto accept_scheduler  = lon::scheduler::IOScheduler::getThis();
        auto process_scheduler = lon::scheduler::IOScheduler::getThis();
        if (!config_server.accept_scheduler.empty())
        {
            accept_scheduler = std::dynamic_pointer_cast<scheduler::IOScheduler>(
                                   SCHEDMGR.getScheduler(config_server.accept_scheduler))
                                   .get();
            if (!accept_scheduler)
            {
                LON_ERROR(g_logger)
                    << "accept_scheduler: " << config_server.accept_scheduler << " not exists";
                exit(0);
            }
        }
        if (!config_server.process_scheduler.empty())
        {
            process_scheduler = std::dynamic_pointer_cast<scheduler::IOScheduler>(
                                    SCHEDMGR.getScheduler(config_server.process_scheduler))
                                    .get();
            if (!process_scheduler)
            {
                LON_ERROR(g_logger)
                    << "process_scheduler: " << config_server.process_scheduler << " not exists";
                exit(0);
            }
        }

        server::TcpServer::Ptr server = server::ServerFactory::Instance().create(
            config_server.type, process_scheduler, accept_scheduler, config_server);
        if (!server)
        {
            LON_WARN(g_logger) << "invalid server type=" << config_server.type;
            server.reset(new server::TcpServer(process_scheduler, accept_scheduler,
                                               G_CONFIG.config_tcp_server_client_timeout->getData(),
                                               config_server.name));
        }

        std::vector<net::Address::Ptr> fails;
        if (!server->bind(addrs, fails, config_server.ssl))
        {
            for (auto &fail : fails)
            {
                LON_ERROR(g_logger) << "bind address fail:" << fail->toString();
            }
            exit(0);
        }
        if (config_server.ssl)
        {
            if (!server->loadCertificates(config_server.cert_file, config_server.key_file))
            {
                LON_ERROR(g_logger)
                    << "loadCertificates fail, cert_file=" << config_server.cert_file
                    << " key_file=" << config_server.key_file;
            }
        }
        server->start();
        m_servers[config_server.type].push_back(server);
    }

    for (const auto &plugin : plugins)
    {
        plugin->onServerReady();
    }
    return 0;
}

} // namespace system
} // namespace lon