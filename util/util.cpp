#include "util/util.h"

namespace lon
{
namespace util
{
char toLower(const char &ch)
{
    if (ch >= 'A' && ch <= 'Z')
    {
        return (char)(ch + 32);
    }
    else
    {
        return ch;
    }
}

std::string toLower(const std::string &str)
{
    std::string res = str;
    for (auto &ch : res)
    {
        ch = toLower(ch);
    }
    return res;
}

char toUpper(const char &ch)
{
    if (ch >= 'a' && ch <= 'z')
    {
        return (char)(ch - 32);
    }
    else
    {
        return ch;
    }
}

std::string toUpper(const std::string &str)
{
    std::string res = str;
    for (auto &ch : res)
    {
        ch = toUpper(ch);
    }
    return res;
}

bool isValidParamName(const std::string &str)
{
    if (str.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.") !=
        std::string::npos)
    {
        return false;
    }
    return true;
}

void printYamlString(const YAML::Node &node, int layer)
{
    if (node.IsNull())
    {
        std::stringstream ss;
        ss << std::string(layer * 2, ' ') << "Null -" << node.Type() << "-" << layer << std::endl;
        std::cout << ss.str();
    }
    else if (node.IsScalar())
    {
        std::stringstream ss;
        ss << std::string(layer * 2, ' ') << node.Scalar() << "-" << node.Type() << "-" << layer
           << std::endl;
        std::cout << ss.str();
    }
    else if (node.IsMap())
    {
        for (auto it = node.begin(); it != node.end(); ++it)
        {
            std::stringstream ss;
            ss << std::string(layer * 2, ' ') << it->first << "-" << it->second.Type() << "-"
               << layer << std::endl;
            std::cout << ss.str();
            printYamlString(it->second, layer + 1);
        }
    }
    else if (node.IsSequence())
    {
        for (int cnt = 0; cnt < node.size(); cnt++)
        {
            std::stringstream ss;
            ss << std::string(layer * 2, ' ') << cnt << "-" << node[cnt].Type() << "-" << layer
               << std::endl;
            std::cout << ss.str();
            printYamlString(node[cnt], layer + 1);
        }
    }
}

void convertYamlToVector(const std::string &prefix, const YAML::Node &node,
                         std::vector<std::pair<std::string, YAML::Node>> &vec)
{
    if (!isValidParamName(toLower(prefix)))
    {
        throw std::runtime_error("data name is invalid: " + prefix);
    }
    vec.push_back(std::make_pair(toLower(prefix), node));
    if (node.IsMap())
    {
        for (auto it = node.begin(); it != node.end(); ++it)
        {
            convertYamlToVector(prefix.empty()
                                    ? it->first.Scalar()
                                    : std::string(toLower(prefix) + "." + it->first.Scalar()),
                                it->second, vec);
        }
    }
}

void getColorStr(std::string &str, Color color)
{
    auto color_num = (int)color;
    if (color_num < 1)
    {
        return;
    }
    std::string front = "\033[" + std::to_string(29 + color_num) + "m";
    std::string back  = "\033[0m";
    str               = front + str + back;
}

std::vector<std::unordered_map<std::string, uint8_t>> formatParser(const std::string &str)
{
    std::vector<std::unordered_map<std::string, uint8_t>> res;
    for (int i = 0; i < str.size(); i++)
    {
        if (str[i] != '%')
        {
            std::string temp = "";
            while (str[i] != '%' && i < str.size())
            {
                temp += str[i];
                i++;
            }
            std::unordered_map<std::string, uint8_t> m;
            m[temp] = 0;
            res.push_back(m);
            i--;
        }
        else
        {
            if ((i + 1) < str.size() && str[i + 1] != '%')
            {
                if ((i + 2) < str.size() && str[i + 1] == 'd' && str[i + 2] == '{')
                {
                    i += 3;
                    std::string temp = "";
                    while (str[i] != '}' && i < str.size())
                    {
                        temp += str[i];
                        i++;
                    }
                    if (i >= str.size())
                    {
                        throw std::runtime_error(temp + ": format error");
                        break;
                    }
                    std::unordered_map<std::string, uint8_t> m;
                    m[temp] = 2;
                    res.push_back(m);
                }
                else
                {
                    std::unordered_map<std::string, uint8_t> m;
                    std::string temp = "";
                    temp += str[i + 1];
                    m[temp] = 1;
                    res.push_back(m);
                    i++;
                }
            }
            else
            {
                std::unordered_map<std::string, uint8_t> m;
                m["%"] = 0;
                res.push_back(m);
            }
        }
    }
    return res;
}

std::string getDateTime(const time_t &time, const std::string &format)
{
    struct tm tm;
    localtime_r(&time, &tm);
    char buf[64] = {0};
    strftime(buf, sizeof(buf), format.c_str(), &tm);
    return std::string(buf);
}

time_t getCurrentDateTime() { return time(nullptr); }

std::string getCurrentDateTime(const std::string &format)
{
    return getDateTime(getCurrentDateTime(), format);
}

uint32_t getThreadId() { return syscall(SYS_gettid); }

std::string getThreadName()
{
    char buf[16] = {0};
    pthread_getname_np(pthread_self(), buf, sizeof(buf));
    return std::string(buf);
}

// TODO - 实现获取协程id
uint32_t getFiberId() { return 0; }
} // namespace util
} // namespace lon