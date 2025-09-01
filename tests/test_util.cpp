#include "lonetfw/lonetfw.h"

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

int main(int argc, char const *argv[])
{
    lon::config::Config::parseFromYaml("./.config/log.yaml");
    // test_backtrace();
    test_assert();
    return 0;
}
