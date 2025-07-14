#include "config/config.h"
#include "log/logger.h"
#include <chrono>

void test_log()
{
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100000; i++)
    {
        LON_INFO(LON_LOG_NAME("root")) << "test";
    }
    auto end = std::chrono::high_resolution_clock::now();

    LON_INFO(LON_LOG_ROOT)
        << "duration: "
        << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "us";
}

int main(int argc, char const *argv[])
{
    lon::config::Config::parseFromYaml("./.config/log.yaml");
    test_log();
    return 0;
}
