#pragma once
#include "log/logger.h"
#include "util/util.h"
namespace lon
{
namespace config
{
struct ConfigDataBase
{
  public:
    using Ptr = std::shared_ptr<ConfigDataBase>;
    explicit ConfigDataBase(const std::string name, const std::string description = "");
    virtual ~ConfigDataBase()                       = default;
    virtual std::string toString()                  = 0;
    virtual bool fromString(const std::string &str) = 0;
    virtual std::string getType() const             = 0;

    std::string getName() const;
    std::string getDescription() const;

  private:
    std::string m_name;
    std::string m_description;
};

template <typename T, class FromStr = util::LexicalCast<T, std::string>,
          class ToStr = util::LexicalCast<std::string, T>>
struct ConfigData : public ConfigDataBase
{
  public:
    using Ptr                  = std::shared_ptr<ConfigData<T>>;
    using onConfigDataChangeCB = std::function<void(const T &old_data, const T &new_data)>;

    ConfigData(const std::string &name, const T &data, const std::string &description = "")
        : ConfigDataBase(name, description), m_data(data){};
    ~ConfigData() = default;

    std::string getType() const override { return util::getTypeStr<T>(); }
    std::string toString() override
    {
        try
        {
            return ToStr()(getData());
        }
        catch (const std::runtime_error &e)
        {
            LON_ERROR(LON_LOG_ROOT) << e.what();
            return std::string();
        }
    }
    bool fromString(const std::string &str) override
    {
        try
        {
            setData(FromStr()(str));
            return true;
        }
        catch (const std::runtime_error &e)
        {
            LON_ERROR(LON_LOG_ROOT) << e.what() << ": " << getType() << "; msg:[" << str << "]";
            return false;
        }
    }
    T getData() const { return m_data; }
    void setData(const T &data)
    {
        if (m_data == data)
        {
            return;
        }
        for (const auto &it : m_cbs)
        {
            it.second(m_data, data);
        }
        m_data = data;
    }

    bool addConfigDataChangeCB(uint64_t key, onConfigDataChangeCB cb)
    {
        auto it = m_cbs.find(key);
        if (it != m_cbs.end())
        {
            LON_WARN(LON_LOG_ROOT) << "addConfigDataChangeCB: key is exists: " << key;
            return false;
        }
        m_cbs[key] = cb;
        return true;
    }

    bool delConfigDataChangeCB(const uint64_t &key)
    {
        auto it = m_cbs.find(key);
        if (it != m_cbs.end())
        {
            LON_WARN(LON_LOG_ROOT) << "delConfigDataChangeCB: key is not exists: " << key;
            return false;
        }
        m_cbs.erase(it);
        return true;
    }

    onConfigDataChangeCB getConfigDataChangeCB(const uint64_t &key)
    {
        auto it = m_cbs.find(key);
        if (it != m_cbs.end())
        {
            LON_WARN(LON_LOG_ROOT) << "getConfigDataChangeCB: key is not exists: " << key;
            return nullptr;
        }
        return it->second;
    }

    void clearConfigDataChangeCB() { m_cbs.clear(); }

  private:
    T m_data;
    std::map<uint64_t, onConfigDataChangeCB> m_cbs;
};

} // namespace config
} // namespace lon