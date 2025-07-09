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
} // namespace util
} // namespace lon