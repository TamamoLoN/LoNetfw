#pragma once
#include "config/configdata.h"
namespace lon
{
namespace config
{
class Config
{
  public:
    using ConfigDataMap = std::map<std::string, ConfigDataBase::Ptr>;
    using Ptr           = std::shared_ptr<Config>;
    explicit Config()   = default;
    virtual ~Config()   = default;

    //创建并设置当前Config数据，返回数据实例
    template <typename T>
    static ConfigDataBase::Ptr setData(const std::string &name, const T &data,
                                       const std::string &description = "")
    {
        auto name_lower = util::toLower(name);
        if (!util::isValidParamName(name_lower))
        {
            LON_ERROR(LON_LOG_ROOT) << "data name is invalid: " << name;
            return nullptr;
        }
        auto data_ptr       = std::make_shared<ConfigData<T>>(name, data, description);
        s_datas[name_lower] = data_ptr;
        return data_ptr;
    }

    static void setData(const ConfigDataBase::Ptr &data_ptr)
    {
        auto name_lower = util::toLower(data_ptr->getName());
        if (!util::isValidParamName(name_lower))
        {
            LON_ERROR(LON_LOG_ROOT) << "data name is invalid: " << data_ptr->getName();
            return;
        }
        s_datas[name_lower] = data_ptr;
    }

    template <typename T> static typename ConfigData<T>::Ptr getData(const std::string &name)
    {
        auto name_lower = util::toLower(name);
        auto it         = s_datas.find(name_lower);
        if (!util::isValidParamName(name_lower))
        {
            LON_ERROR(LON_LOG_ROOT) << "ConfigData<T>::Ptr getData: data name is invalid: " << name;
            return nullptr;
        }
        if (it == s_datas.end())
        {
            LON_WARN(LON_LOG_ROOT) << "ConfigData<T>::Ptr getData: cannot find data: " << name;
            return nullptr;
        }
        return std::dynamic_pointer_cast<ConfigData<T>>(it->second);
    }

    static ConfigDataBase::Ptr getDataBase(const std::string &name);

    static void parseFromYaml(const std::string &yaml_path);
    static void parseFromYaml(YAML::Node node);

  private:
    static ConfigDataMap s_datas;
};
} // namespace config
} // namespace lon