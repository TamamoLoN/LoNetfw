#pragma once

#include "yaml-cpp/yaml.h"
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace lon
{

namespace util
{
template <typename T, typename S> T lexical_cast(const S &source)
{
    std::stringstream ss;
    T res;
    if (!(ss << source) || !(ss >> res) || !(ss >> std::ws).eof())
    {
        throw std::runtime_error("error cast: [" + ss.str() + "]");
    }

    return res;
}

// T：输出类型 S：输入类型
template <typename T, typename S> class LexicalCast
{
  public:
    T operator()(const S &source) const { return lexical_cast<T>(source); }
};

template <typename T> class LexicalCast<std::vector<T>, std::string>
{
  public:
    std::vector<T> operator()(const std::string &source) const
    {
        YAML::Node node = YAML::Load(source);
        std::vector<T> res;
        for (int cnt = 0; cnt < node.size(); cnt++)
        {
            res.push_back(LexicalCast<T, std::string>()(node[cnt].as<std::string>()));
        }
        return res;
    }
};

template <typename T> class LexicalCast<std::string, std::vector<T>>
{
  public:
    std::string operator()(std::vector<T> &source) const
    {
        YAML::Node node;
        for (int cnt = 0; cnt < source.size(); cnt++)
        {
            node.push_back(LexicalCast<std::string, T>()(source[cnt]));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template <typename T> class LexicalCast<std::list<T>, std::string>
{
  public:
    std::list<T> operator()(const std::string &source) const
    {
        YAML::Node node = YAML::Load(source);
        std::list<T> res;
        for (int cnt = 0; cnt < node.size(); cnt++)
        {
            res.push_back(LexicalCast<T, std::string>()(node[cnt].as<std::string>()));
        }
        return res;
    }
};

template <typename T> class LexicalCast<std::string, std::list<T>>
{
  public:
    std::string operator()(std::list<T> &source) const
    {
        YAML::Node node;
        for (const auto &it : source)
        {
            node.push_back(LexicalCast<std::string, T>()(it));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template <typename T> class LexicalCast<std::set<T>, std::string>
{
  public:
    std::set<T> operator()(const std::string &source) const
    {
        YAML::Node node = YAML::Load(source);
        std::set<T> res;
        for (int cnt = 0; cnt < node.size(); cnt++)
        {
            res.insert(LexicalCast<T, std::string>()(node[cnt].as<std::string>()));
        }
        return res;
    }
};

template <typename T> class LexicalCast<std::string, std::set<T>>
{
  public:
    std::string operator()(std::set<T> &source) const
    {
        YAML::Node node;
        for (const auto &it : source)
        {
            node.push_back(LexicalCast<std::string, T>()(it));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template <typename T> class LexicalCast<std::unordered_set<T>, std::string>
{
  public:
    std::unordered_set<T> operator()(const std::string &source) const
    {
        YAML::Node node = YAML::Load(source);
        std::unordered_set<T> res;
        for (int cnt = 0; cnt < node.size(); cnt++)
        {
            res.insert(LexicalCast<T, std::string>()(node[cnt].as<std::string>()));
        }
        return res;
    }
};

template <typename T> class LexicalCast<std::string, std::unordered_set<T>>
{
  public:
    std::string operator()(std::unordered_set<T> &source) const
    {
        YAML::Node node;
        for (const auto &it : source)
        {
            node.push_back(LexicalCast<std::string, T>()(it));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template <typename T> class LexicalCast<std::map<std::string, T>, std::string>
{
  public:
    std::map<std::string, T> operator()(const std::string &source) const
    {
        YAML::Node node = YAML::Load(source);
        std::map<std::string, T> res;
        for (auto it = node.begin(); it != node.end(); ++it)
        {
            res[it->first.Scalar()] = (LexicalCast<T, std::string>()(it->second.as<std::string>()));
        }
        return res;
    }
};

template <typename T> class LexicalCast<std::string, std::map<std::string, T>>
{
  public:
    std::string operator()(std::map<std::string, T> &source) const
    {
        YAML::Node node;
        for (const auto &it : source)
        {
            node[it.first] = YAML::Load(LexicalCast<std::string, T>()(it.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template <typename T> class LexicalCast<std::unordered_map<std::string, T>, std::string>
{
  public:
    std::unordered_map<std::string, T> operator()(const std::string &source) const
    {
        YAML::Node node = YAML::Load(source);
        std::unordered_map<std::string, T> res;
        for (auto it = node.begin(); it != node.end(); ++it)
        {
            res[it->first.Scalar()] = (LexicalCast<T, std::string>()(it->second.as<std::string>()));
        }
        return res;
    }
};

template <typename T> class LexicalCast<std::string, std::unordered_map<std::string, T>>
{
  public:
    std::string operator()(std::unordered_map<std::string, T> &source) const
    {
        YAML::Node node;
        for (const auto &it : source)
        {
            node[it.first] = YAML::Load(LexicalCast<std::string, T>()(it.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

} // namespace util
} // namespace lon