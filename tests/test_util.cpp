#include "lonetfw/lonetfw.h"

void test_parse_log_format()
{
    auto res =
        lon::util::formatParser("%d{%Y-%m-%d %H:%M:%S}中文%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n");
    for (const auto &i : res)
    {
        for (const auto &j : i)
        {
            std::cout << j.first << ":" << (int)j.second << "\n";
        }
    }
}

void test_lexical_cast()
{
    auto vec = std::vector<std::map<std::string, int>>{
        {{"a", 1}, {"b", 2}},
        {{"c", 3}, {"d", 4}},
    };

    LON_INFO(LON_LOG_NAME("root"))
        << lon::util::LexicalCast<std::string, std::vector<std::map<std::string, int>>>()(vec);
}

void test_backtrace()
{
    try
    {
        throw std::runtime_error("test");
    }
    catch (const std::exception &e)
    {
        LON_ERROR(LON_LOG_ROOT) << e.what() << '\n' << lon::util::backtrace(100, 2, "\t");
    }
}

void test_assert() { LON_ASSERT_(1 == 1, "hello world"); }

void test_zigzag()
{
    int16_t a         = -1;
    uint16_t a_encode = lon::util::ZigZag::encode16(a);
    LON_INFO(LON_LOG_ROOT) << "a: " << a;
    LON_INFO(LON_LOG_ROOT) << "a_encode: " << a_encode;
    LON_INFO(LON_LOG_ROOT) << "a_encode decode: " << lon::util::ZigZag::decode16(a_encode);
}

int main(int argc, char const *argv[])
{
    lon::config::Config::parseFromYaml("./.config/log.yaml");
    // test_backtrace();
    // test_assert();
    test_parse_log_format();
    test_lexical_cast();
    test_zigzag();
    return 0;
}
