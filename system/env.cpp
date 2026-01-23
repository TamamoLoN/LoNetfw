#include "system/env.h"

namespace lon
{
namespace system
{
bool Env::init(int argc, char **argv)
{
    char link[1024] = {0};
    char path[1024] = {0};
    sprintf(link, "/proc/%d/exe", getpid());
    if (LON_UNLIKELY(readlink(link, path, sizeof(path)) == -1))
    {
        return false;
    }
    // /path/xxx/exe
    m_exe = path;

    auto pos = m_exe.find_last_of("/");
    m_cwd    = m_exe.substr(0, pos) + "/";

    m_program = argv[0];
    // -config /path/to/config -file xxxx -d
    try
    {
        m_argparser.parse(argc, argv);
    }
    catch (...)
    {
        return false;
    }

    return true;
}

const std::string &Env::getExe() const { return m_exe; }

const std::string &Env::getCwd() const { return m_cwd; }

bool Env::setEnv(const std::string &key, const std::string &val)
{
    return !setenv(key.c_str(), val.c_str(), 1);
}

std::string Env::getEnv(const std::string &key, const std::string &default_value)
{
    const char *v = getenv(key.c_str());
    if (v == nullptr)
    {
        return default_value;
    }
    return v;
}

std::string Env::getAbsolutePath(const std::string &path) const
{
    if (path.empty())
    {
        return "/";
    }
    if (path[0] == '/')
    {
        return path;
    }
    return m_cwd + path;
}

std::string Env::getConfigPath() { return getAbsolutePath(get<std::string>("--config")); }

util::ArgumentParser &Env::getArgParser()
{
    MutexType::WrLock lock(m_mutex);
    return m_argparser;
}

util::Argument::Ptr Env::addArgument(const std::string &name)
{
    MutexType::WrLock lock(m_mutex);
    return m_argparser.addArgument(name);
}

util::Argument::Ptr Env::addArgument(const std::vector<std::string> &names)
{
    MutexType::WrLock lock(m_mutex);
    return m_argparser.addArgument(names);
}

} // namespace system
} // namespace lon