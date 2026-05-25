#include "system/env.h"

#ifdef _WIN32
#include <windows.h>

#include <process.h>
#else
#include <unistd.h>
#endif

namespace lon
{
namespace system
{
bool Env::init(int argc, char **argv)
{
#ifdef _WIN32
    char path[MAX_PATH] = {0};
    DWORD len = GetModuleFileNameA(NULL, path, MAX_PATH);
    if (len == 0 || len == MAX_PATH)
    {
        return false;
    }
    m_exe = path;
    std::replace(m_exe.begin(), m_exe.end(), '\\', '/');
#else
    char link[1024] = {0};
    char path[1024] = {0};
    sprintf(link, "/proc/%d/exe", getpid());
    if (LON_UNLIKELY(readlink(link, path, sizeof(path)) == -1))
    {
        return false;
    }
    // /path/xxx/exe
    m_exe = path;
#endif

    auto pos = m_exe.find_last_of("/");
    m_cwd    = m_exe.substr(0, pos) + "/";

    m_program = argv[0];
    std::replace(m_program.begin(), m_program.end(), '\\', '/');
    pos       = m_program.find_last_of("/");
    if (pos != std::string::npos)
    {
        m_program = m_program.substr(pos + 1);
    }
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

const std::string &Env::getProgram() const { return m_program; }

bool Env::setEnv(const std::string &key, const std::string &val)
{
#ifdef _WIN32
    return _putenv_s(key.c_str(), val.c_str()) == 0;
#else
    return !setenv(key.c_str(), val.c_str(), 1);
#endif
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

std::string Env::getPluginPath() { return getAbsolutePath(get<std::string>("--plugin")); }

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