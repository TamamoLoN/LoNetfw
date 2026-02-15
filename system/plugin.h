#pragma once

#include "config/config.h"
#include "log/logger.h"
#include "system/env.h"
#include "thread/mutex.h"
#include "util/singleton.h"
#include "util/stream.h"
#include <dlfcn.h>
#include <unordered_map>

namespace lon
{
namespace system
{
class Plugin
{
  public:
    using Ptr = std::shared_ptr<Plugin>;
    Plugin(const std::string &name, const std::string &version, const std::string &path);
    virtual ~Plugin();

    virtual void onBeforeArgParse(int argc, char **argv);
    virtual void onAfterArgParse(int argc, char **argv);

    virtual bool onLoad();
    virtual bool onUnload();

    virtual bool onConnect(const util::Stream::Ptr &stream);
    virtual bool onDisconnect(const util::Stream::Ptr &stream);

    virtual bool onServerReady();
    virtual bool onServerUp();

    virtual std::string statusString();

    const std::string &getName() const;
    const std::string &getVersion() const;
    const std::string &getPath() const;
    const std::string &getId() const;

    static Plugin::Ptr loadPlugin(const std::string &path);

  protected:
    std::string m_name;
    std::string m_version;
    std::string m_path;
    std::string m_id;
};

#define PLUGINMGR lon::util::Singleton<lon::system::PluginManager>::Instance()

class PluginManager
{
  public:
    using MutexType = lon::thread::RWMutex;
    PluginManager();
    virtual ~PluginManager();

    void add(const Plugin::Ptr &plugin);
    void del(const std::string &name);
    void delAll();
    void init(const std::string &plugin_path);
    Plugin::Ptr get(const std::string &name);
    void getAll(std::vector<Plugin::Ptr> &plugins);

    void onConnect(const util::Stream::Ptr &stream);
    void onDisconnect(const util::Stream::Ptr &stream);

  private:
    void loadPlugin(const std::string &path);

  private:
    MutexType m_mutex;
    std::unordered_map<std::string, Plugin::Ptr> m_plugins;
};
} // namespace system
} // namespace lon