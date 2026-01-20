#include "config/config.h"

namespace lon
{
namespace config
{
ConfigDataBase::Ptr Config::getDataBase(const std::string &name)
{
    if (getDatas().find(name) != getDatas().end())
    {
        return getDatas()[name];
    }
    return nullptr;
}

void Config::parseFromYaml(const std::string &yaml_path)
{
    {
        // TODO - 根据md5判断文件是否修改过，如果没有修改过则不重新加载
        // struct stat st;
        // lstat(i.c_str(), &st);
        // sylar::Mutex::Lock lock(s_mutex);
        // if (s_file2modifytime[i] == (uint64_t)st.st_mtime)
        // {
        //     continue;
        // }
        // s_file2modifytime[i] = st.st_mtime;
    }
    YAML::Node root = YAML::LoadFile(yaml_path);
    parseFromYaml(root);
}

void Config::parseFromYaml(YAML::Node node)
{
    std::vector<std::pair<std::string, YAML::Node>> all_nodes;
    try
    {
        util::convertYamlToVector("", node, all_nodes);
        for (const auto &node : all_nodes)
        {
            auto key = node.first;
            if (key.empty())
            {
                continue;
            }
            auto data_base = getDataBase(key);
            if (data_base == nullptr)
            {
                LON_DEBUG(LON_LOG_ROOT) << "Config::parseFromYaml: "
                                        << "cannot find data: " << key;
            }
            else
            {
                if (node.second.IsScalar())
                {
                    data_base->fromString(node.second.Scalar());
                }
                else
                {
                    std::stringstream ss;
                    ss << node.second;
                    data_base->fromString(ss.str());
                }
            }
        }
    }
    catch (const std::runtime_error &e)
    {
        LON_ERROR(LON_LOG_ROOT) << e.what();
    }
}

void Config::parseFromDir(const std::string &dir_path)
{
    std::vector<std::string> files;
    util::FSUtil::getDirFiles(files, dir_path, ".yaml");
    for (auto &file : files)
    {
        try
        {
            parseFromYaml(file);
            LON_INFO(LON_LOG_ROOT) << "parseFromDir config file=" << file << " success";
        }
        catch (...)
        {
            LON_ERROR(LON_LOG_ROOT) << "parseFromDir config file=" << file << " failed";
        }
    }
}

void Config::visit(std::function<void(config::ConfigDataBase::Ptr)> cb)
{
    thread::RWMutex::RdLock lock(getMutex());
    for (const auto &it : getDatas())
    {
        cb(it.second);
    }
}

std::ostream &Config::toString(std::ostream &os)
{
    thread::RWMutex::RdLock lock(getMutex());
    for (const auto &it : getDatas())
    {
        os << it.first << ":\n\t" << it.second->toString() << "\n";
    }
    return os;
}

std::string Config::toString()
{
    std::stringstream ss;
    toString(ss);
    return ss.str();
}

} // namespace config
} // namespace lon