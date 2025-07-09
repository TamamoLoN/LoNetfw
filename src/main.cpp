#include "log/logger.h"
#include "util/util.h"
using namespace std;
using namespace lon;
using namespace log;
using namespace util;

template <typename T> void print(T value) { cout << "end:" << value << endl; }
template <typename T, typename... Args> void print(const T &value, Args... args)
{
    cout << value << endl;
    print(args...);
}

void test(std::string m_pattern)
{
    std::vector<std::tuple<std::string, std::string, int>> vec;
    std::string nstr;
    for (size_t i = 0; i < m_pattern.size(); ++i)
    {
        if (m_pattern[i] != '%')
        {
            nstr.append(1, m_pattern[i]);
            continue;
        }

        if ((i + 1) < m_pattern.size())
        {
            if (m_pattern[i + 1] == '%')
            {
                nstr.append(1, '%');
                continue;
            }
        }

        size_t n         = i + 1;
        int fmt_status   = 0;
        size_t fmt_begin = 0;

        std::string str;
        std::string fmt;
        while (n < m_pattern.size())
        {
            if (!fmt_status &&
                (!isalpha(m_pattern[n]) && m_pattern[n] != '{' && m_pattern[n] != '}'))
            {
                str = m_pattern.substr(i + 1, n - i - 1);
                break;
            }
            if (fmt_status == 0)
            {
                if (m_pattern[n] == '{')
                {
                    str = m_pattern.substr(i + 1, n - i - 1);
                    // std::cout << "*" << str << std::endl;
                    fmt_status = 1; //解析格式
                    fmt_begin  = n;
                    ++n;
                    continue;
                }
            }
            else if (fmt_status == 1)
            {
                if (m_pattern[n] == '}')
                {
                    fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
                    // std::cout << "#" << fmt << std::endl;
                    fmt_status = 0;
                    ++n;
                    break;
                }
            }
            ++n;
            if (n == m_pattern.size())
            {
                if (str.empty())
                {
                    str = m_pattern.substr(i + 1);
                }
            }
        }

        if (fmt_status == 0)
        {
            if (!nstr.empty())
            {
                vec.push_back(std::make_tuple(nstr, std::string(), 0));
                nstr.clear();
            }
            vec.push_back(std::make_tuple(str, fmt, 1));
            i = n - 1;
        }
        else if (fmt_status == 1)
        {
            std::cout << "pattern parse error: " << m_pattern << " - " << m_pattern.substr(i)
                      << std::endl;
            vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
        }
    }

    if (!nstr.empty())
    {
        vec.push_back(std::make_tuple(nstr, "", 0));
    }
    // 添加在函数末尾（for 循环之后）
    for (const auto &item : vec)
    {
        const std::string &str = std::get<0>(item);
        const std::string &fmt = std::get<1>(item);
        int type               = std::get<2>(item);

        std::cout << "[Item] type=" << type << ", str=\"" << str << "\""
                  << ", fmt=\"" << fmt << "\"" << std::endl;
    }
}

int main(int argc, char const *argv[])
{
    auto l = std::make_shared<Logger>("test");
    l->addAppender(std::make_shared<StdoutLogAppender>());
    auto e = std::make_shared<LogEvent>(std::string(__FILE__), __LINE__, 0, 1, 2, time(0));
    e->getMessageStream() << "hello lon log";
    l->log(Loglevel::Level::DEBUG, e);
    // // l.debug()
    // l.test();
    // print(1, "hello", 11.2);
    // auto res = formatParser("(%a) %%%%%  %a%b %v");
    // for (const auto &i : res)
    // {
    //     for (const auto &j : i)
    //     {
    //         cout << j.first << ":"
    //              << [=](bool b) -> std::string { return b ? "true" : "false"; }(j.second) <<
    //              endl;
    //     }
    // }

    // test("(%a) %%%%  %n%b %v");
    std::cout << getCurrentDateTime("%Y-%m-%d %H:%M:%S");
    return 0;
}
