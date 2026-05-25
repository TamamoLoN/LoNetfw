#pragma once

#include "log/logger.h"
#include "thread/mutex.h"
#include "util/util.h"

namespace lon
{
namespace system
{
class LON_API Env
{
  public:
    using MutexType = thread::RWMutex;
    bool init(int argc, char **argv);

    const std::string &getExe() const;
    const std::string &getCwd() const;
    const std::string &getProgram() const;

    bool setEnv(const std::string &key, const std::string &val);
    std::string getEnv(const std::string &key, const std::string &default_value = "");

    std::string getAbsolutePath(const std::string &path) const;
    std::string getConfigPath();
    std::string getPluginPath();

    util::ArgumentParser &getArgParser();

    util::Argument::Ptr addArgument(const std::string &name);
    util::Argument::Ptr addArgument(const std::vector<std::string> &names);

    template <typename T> T get(const std::string &name)
    {
        MutexType::RdLock lock(m_mutex);
        return m_argparser.get<T>(name);
    }

  private:
    MutexType m_mutex;
    util::ArgumentParser m_argparser;
    std::string m_program; // 可执行文件名
    std::string m_exe;     // 可执行文件实际绝对路径
    std::string m_cwd;
};
#define ENVMGR lon::util::Singleton<lon::system::Env>::Instance()
} // namespace system
} // namespace lon