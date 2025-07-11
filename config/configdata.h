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
    using Ptr = std::shared_ptr<ConfigData<T>>;

    ConfigData(const std::string &name, const T &data, const std::string &description = "")
        : ConfigDataBase(name, description), m_data(data){};
    ~ConfigData() = default;

    std::string toString() override
    {
        try
        {
            return ToStr()(m_data);
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
            m_data = FromStr()(str);
            return true;
        }
        catch (const std::runtime_error &e)
        {
            LON_ERROR(LON_LOG_ROOT) << e.what();
            return false;
        }
    }
    T getData() const { return m_data; }
    void setData(const T &data) { m_data = data; }

  private:
    T m_data;
};

} // namespace config
} // namespace lon