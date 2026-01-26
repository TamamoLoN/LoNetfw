#include "system/daemon.h"

namespace lon
{
namespace system
{
static auto g_logger = LON_LOG_ROOT;

std::string ProcessInfo::toString() const
{
    std::stringstream ss;
    ss << "[ProcessInfo parent_id=" << parent_id << " main_id=" << main_id
       << " parent_start_time=" << util::time2Str(parent_start_time)
       << " main_start_time=" << util::time2Str(main_start_time)
       << " restart_count=" << restart_count << "]";
    return ss.str();
}

static int real_start(int argc, char **argv, std::function<int(int argc, char **argv)> main_cb)
{
    G_PROC_INFO.main_id         = getpid();
    G_PROC_INFO.main_start_time = time(0);
    return main_cb(argc, argv);
}

static int real_daemon(int argc, char **argv, std::function<int(int argc, char **argv)> main_cb)
{
    if (LON_UNLIKELY(daemon(1, 0) == -1))
    {
        LON_ERROR(g_logger) << "daemon fail, errno=" << errno << " errstr=" << strerror(errno);
        return -1;
    }
    G_PROC_INFO.parent_id         = getpid();
    G_PROC_INFO.parent_start_time = time(0);
    while (true)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            //子进程返回
            G_PROC_INFO.main_id         = getpid();
            G_PROC_INFO.main_start_time = time(0);
            LON_INFO(g_logger) << "process start pid=" << getpid();
            return real_start(argc, argv, main_cb);
        }
        else if (pid < 0)
        {
            LON_INFO(g_logger) << "fork fail return=" << pid << " errno=" << errno
                               << " errstr=" << strerror(errno);
            return -1;
        }
        else
        {
            //父进程返回
            int status = 0;
            waitpid(pid, &status, 0);
            if (status)
            {
                if (status == 9)
                {
                    LON_INFO(g_logger) << "killed";
                    break;
                }
                else
                {
                    LON_ERROR(g_logger) << "child crash pid=" << pid << " status=" << status;
                }
            }
            else
            {
                LON_INFO(g_logger) << "child finished pid=" << pid;
                break;
            }
            G_PROC_INFO.restart_count += 1;
            sleep(config::GlobalConfig::Instance().config_system_daemon_restart_delay_s->getData());
        }
    }
    return 0;
}

int start_daemon(int argc, char **argv, std::function<int(int argc, char **argv)> main_cb,
                 bool is_daemon)
{
    if (!is_daemon)
    {
        G_PROC_INFO.parent_id         = getpid();
        G_PROC_INFO.parent_start_time = time(0);
        return real_start(argc, argv, main_cb);
    }
    return real_daemon(argc, argv, main_cb);
}

} // namespace system
} // namespace lon
