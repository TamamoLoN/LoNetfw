#pragma once

#include "util/endian.h"
#include "util/lexicalcast.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace lon
{
namespace util
{
template <typename T> struct is_std_vector : std::false_type
{
};

template <typename T, typename Alloc> struct is_std_vector<std::vector<T, Alloc>> : std::true_type
{
};
// TODO - 不太优雅，后续有时间重构
/**
 * @brief 参考python argparse实现的一个简单命令行参数解析器
 * @note 仅支持部分功能，不支持:
 *  1. 子命令;
 *  2. 互斥参数([--debug | --release]).
 */
class ArgumentParser;
class Argument
{
  public:
    friend class ArgumentParser;
    using Ptr = std::shared_ptr<Argument>;
    Argument *help(const std::string &h);
    Argument *action(const std::string &a);
    Argument *defaultValue(const std::string &d);
    Argument *choices(const std::vector<std::string> &c);
    /**
     * @param nargs:
     *   1. "+" : 1 or more;
     *   2. "*" : 0 or more;
     *   3. "?" : 0 or 1;
     */
    Argument *nargs(const std::string &n);

    bool isMulti() const;
    template <typename T> typename std::enable_if<!is_std_vector<T>::value, T>::type get() const
    {
        return lexical_cast<T>(m_val.at(0));
    }

    template <typename T> typename std::enable_if<is_std_vector<T>::value, T>::type get() const
    {
        typedef typename T::value_type ValueType;
        T ret;
        for (const auto &v : m_val)
        {
            ret.push_back(lexical_cast<ValueType>(v));
        }
        return ret;
    }

  private:
    Argument();
    Argument *name(const std::string &name);
    static std::string toUpper(const std::string &s);
    bool isArgHasValue();
    bool isArgCountValid() const;
    bool isArgChoicesValid() const;
    std::string getHelp(const std::string &name) const;

  private:
    std::string m_name;
    std::vector<std::string> m_val;
    std::string m_help;
    std::string m_action;
    std::vector<std::string> m_choices;
    std::shared_ptr<std::string> m_default;
    std::string m_nargs;
};

class ArgumentParser final
{
  public:
    explicit ArgumentParser();
    ~ArgumentParser();

    void addDescription(const std::string &description);
    Argument::Ptr addArgument(const std::string &name);
    Argument::Ptr addArgument(const std::vector<std::string> &names);
    template <typename T> T get(const std::string &name) const
    {
        auto oit = m_optional_args.find(name);
        if (oit != m_optional_args.end())
        {
            return oit->second->get<T>();
        }
        auto pit = std::find_if(m_positional_args.begin(), m_positional_args.end(),
                                [&](const std::pair<std::string, std::shared_ptr<Argument>> &p) {
                                    return p.first == name;
                                });
        if (pit != m_positional_args.end())
        {
            return pit->second->template get<T>();
        }
        handleError("argument '" + name + "' is not found");
        return T();
    }

    void parse(int argc, char *argv[]);

  private:
    const std::string usage() const;
    const std::string help() const;
    void getArgName(std::string &name);
    void handleError(const std::string &msg) const;
    std::string formatHelper(const std::string &left, const std::string &right, size_t indent = 2,
                             size_t help_col = 26, size_t width = 80) const;

  private:
    std::string m_name;
    std::string m_description;
    std::unordered_map<std::string, Argument::Ptr> m_optional_args;
    std::vector<std::pair<std::string, Argument::Ptr>> m_positional_args;
};
} // namespace util
} // namespace lon