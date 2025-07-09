#include "util.h"

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

std::vector<std::unordered_map<std::string, bool>> formatParser(const std::string &str)
{
    std::vector<std::unordered_map<std::string, bool>> res;
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
            std::unordered_map<std::string, bool> m;
            m[temp] = false;
            res.push_back(m);
            i--;
        }
        else
        {
            if ((i + 1) < str.size() && str[i + 1] != '%')
            {
                std::unordered_map<std::string, bool> m;
                std::string temp = "";
                temp += str[i + 1];
                m[temp] = true;
                res.push_back(m);
                i++;
            }
            else
            {
                std::unordered_map<std::string, bool> m;
                m["%"] = false;
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

} // namespace util
} // namespace lon