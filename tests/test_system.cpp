#include "lonetfw/lonetfw.h"

static auto g_logger = LON_LOG_ROOT;

lon::scheduler::Timer::Ptr timer = nullptr;
int test_daemon(int argc, char **argv)
{
    LON_INFO(g_logger) << G_PROC_INFO.toString();
    lon::scheduler::IOScheduler ios(1);
    timer = ios.addTimer(
        1000,
        []() {
            LON_INFO(g_logger) << "onTimer";
            static int count = 0;
            if (++count > 10)
            {
                exit(1);
            }
        },
        true);
    return 0;
}

int test_argparse(int argc, char *argv[])
{
    lon::util::ArgumentParser parser;
    parser.addDescription("argparse 功能示例程序");
    // 1️.位置参数（必填）
    parser.addArgument("filename")->help("要处理的文件名（必填）");
    // 2️.可选参数，带默认值
    parser.addArgument(std::vector<std::string>{"-c", "--count"})
        ->help("处理次数（可选，默认1次）")
        ->defaultValue("1");
    // 3.布尔开关
    parser.addArgument(std::vector<std::string>{"-v", "--verbose"})
        ->action("store_true")
        ->help("是否显示详细输出");

    // 4️.限定可选值
    parser.addArgument(std::vector<std::string>{"--mode", "-m"})
        ->choices({"easy", "medium", "hard"})
        ->defaultValue("easy")
        ->help("处理模式，可选: easy, medium, hard")
        ->nargs("*");

    // 5.多值参数
    parser.addArgument(std::vector<std::string>{"--tags", "-t"})
        ->nargs("*")
        ->help("标签列表，例如: -t tag1 tag2 tag3");

    try
    {
        parser.parse(argc, argv);
    }
    catch (...)
    {
        return 0;
    }

    auto filename = parser.get<std::vector<std::string>>("filename");
    auto count    = parser.get<std::vector<int>>("--count");
    auto verbose  = parser.get<bool>("-v");
    auto mode     = parser.get<std::string>("--mode");
    auto tags     = parser.get<std::vector<int>>("--tags");

    std::cout << "filename=" << std::endl;
    for (auto &f : filename)
    {
        std::cout << f << " ";
    }
    std::cout << "\ncount=" << std::endl;
    for (auto &c : count)
    {
        std::cout << c << " ";
    }

    std::cout << "\nverbose=" << verbose << std::endl;
    std::cout << "mode=" << mode << std::endl;
    std::cout << "tags=" << std::endl;
    for (auto &t : tags)
    {
        std::cout << t << " ";
    }
    std::cout << std::endl;
    return 0;
}

int test_env(int argc, char *argv[])
{
    if (!ENVMGR.init(argc, argv))
    {
        return 0;
    }
    LON_INFO(g_logger) << "exe=" << ENVMGR.getExe();
    LON_INFO(g_logger) << "cwd=" << ENVMGR.getCwd();
    LON_INFO(g_logger) << "env PATH=" << ENVMGR.getEnv("PATH");
    LON_INFO(g_logger) << "env PATH=" << ENVMGR.getEnv("TEST");
    ENVMGR.setEnv("TEST", "888");
    LON_INFO(g_logger) << "env PATH=" << ENVMGR.getEnv("TEST");
    return 0;
}

int main(int argc, char *argv[])
{
    lon::config::Config::parseFromYaml(".config/log.yaml");
    // return lon::system::start_daemon(argc, argv, test_daemon, true);
    // return test_argparse(argc, argv);
    return test_env(argc, argv);
    return 0;
}
