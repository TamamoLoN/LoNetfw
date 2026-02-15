#include "system/plugin.h"

namespace lon
{
namespace system
{
static auto g_logger = LON_LOG_ROOT;

typedef Plugin *(*CreatePluginType)();
typedef void (*DestroyPluginType)(Plugin *);

class PluginCloser
{
  public:
    PluginCloser(void *handle, DestroyPluginType d) : m_handle(handle), m_destory(d) {}

    void operator()(Plugin *module)
    {
        std::string name    = module->getName();
        std::string version = module->getVersion();
        std::string path    = module->getPath();
        m_destory(module);
        int rt = dlclose(m_handle);
        if (rt)
        {
            LON_ERROR(g_logger) << "dlclose handle fail handle=" << m_handle << " name=" << name
                                << " version=" << version << " path=" << path
                                << " error=" << dlerror();
        }
        else
        {
            LON_INFO(g_logger) << "destory plugin=" << name << " version=" << version
                               << " path=" << path << " handle=" << m_handle << " success";
        }
    }

  private:
    void *m_handle;
    DestroyPluginType m_destory;
};

Plugin::Plugin(const std::string &name, const std::string &version, const std::string &path)
    : m_name(name), m_version(version), m_path(path), m_id(name + "/" + version)
{
}

Plugin::~Plugin() {}

void Plugin::onBeforeArgParse(int argc, char **argv) {}

void Plugin::onAfterArgParse(int argc, char **argv) {}

bool Plugin::onLoad() { return true; }

bool Plugin::onUnload() { return true; }

bool Plugin::onConnect(const util::Stream::Ptr &stream) { return true; }

bool Plugin::onDisconnect(const util::Stream::Ptr &stream) { return true; }

bool Plugin::onServerReady() { return true; }

bool Plugin::onServerUp() { return true; }

std::string Plugin::statusString() { return ""; }

const std::string &Plugin::getName() const { return m_name; }

const std::string &Plugin::getVersion() const { return m_version; }

const std::string &Plugin::getPath() const { return m_path; }

const std::string &Plugin::getId() const { return m_id; }

Plugin::Ptr Plugin::loadPlugin(const std::string &path)
{
    void *handle = dlopen(path.c_str(), RTLD_NOW);
    if (!handle)
    {
        LON_ERROR(g_logger) << "cannot load plugin path=" << path << " error=" << dlerror();
        return nullptr;
    }

    CreatePluginType create = (CreatePluginType)dlsym(handle, "CreatePlugin");
    if (!create)
    {
        LON_ERROR(g_logger) << "cannot load symbol CreatePlugin in " << path
                            << " error=" << dlerror();
        dlclose(handle);
        return nullptr;
    }

    DestroyPluginType destory = (DestroyPluginType)dlsym(handle, "DestoryPlugin");
    if (!destory)
    {
        LON_ERROR(g_logger) << "cannot load symbol DestoryPlugin in " << path
                            << " error=" << dlerror();
        dlclose(handle);
        return nullptr;
    }

    Plugin::Ptr plugin(create(), PluginCloser(handle, destory));
    plugin->m_path = path;
    LON_INFO(g_logger) << "load plugin name=" << plugin->getName()
                       << " version=" << plugin->getVersion() << " path=" << plugin->getPath()
                       << " success";
    config::Config::parseFromDir(ENVMGR.getConfigPath());
    return plugin;
}

PluginManager::PluginManager() {}

PluginManager::~PluginManager() {}

void PluginManager::add(const Plugin::Ptr &plugin)
{
    if (!plugin)
    {
        return;
    }
    del(plugin->getId());
    MutexType::WrLock lock(m_mutex);
    m_plugins[plugin->getId()] = plugin;
}

void PluginManager::del(const std::string &name)
{
    Plugin::Ptr plugin = nullptr;
    MutexType::WrLock lock(m_mutex);
    auto it = m_plugins.find(name);
    if (it == m_plugins.end())
    {
        return;
    }
    plugin = it->second;
    m_plugins.erase(it);
    lock.unlock();
    plugin->onUnload();
}

void PluginManager::delAll()
{
    MutexType::RdLock lock(m_mutex);
    auto plugins = m_plugins;
    lock.unlock();

    for (const auto &plugin : plugins)
    {
        del(plugin.first);
    }
}

void PluginManager::init()
{
    auto plugin_path = ENVMGR.getPluginPath();
    std::vector<std::string> plugins{};
    util::FSUtil::getDirFiles(plugins, plugin_path, ".so");

    std::sort(plugins.begin(), plugins.end());
    for (const auto &plugin : plugins)
    {
        loadPlugin(plugin);
    }
}

Plugin::Ptr PluginManager::get(const std::string &name)
{
    MutexType::RdLock lock(m_mutex);
    auto it = m_plugins.find(name);
    return it == m_plugins.end() ? nullptr : it->second;
}

void PluginManager::getAll(std::vector<Plugin::Ptr> &plugins)
{
    MutexType::RdLock lock(m_mutex);
    for (const auto &plugin : m_plugins)
    {
        plugins.push_back(plugin.second);
    }
}

void PluginManager::onConnect(const util::Stream::Ptr &stream)
{
    std::vector<Plugin::Ptr> plugins{};
    getAll(plugins);
    for (const auto &plugin : plugins)
    {
        plugin->onConnect(stream);
    }
}

void PluginManager::onDisconnect(const util::Stream::Ptr &stream)
{
    std::vector<Plugin::Ptr> plugins{};
    getAll(plugins);
    for (const auto &plugin : plugins)
    {
        plugin->onDisconnect(stream);
    }
}

void PluginManager::loadPlugin(const std::string &path)
{
    auto plugin = Plugin::loadPlugin(path);
    if (!plugin)
    {
        return;
    }
    add(plugin);
}
} // namespace system
} // namespace lon