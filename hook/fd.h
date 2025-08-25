#pragma once
#include "scheduler/ioscheduler.h"
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace lon
{
namespace hook
{
class Fd : public std::enable_shared_from_this<Fd>
{
  public:
    using Ptr         = std::shared_ptr<Fd>;
    using TimeoutType = scheduler::IOScheduler::Event;
    Fd(int fd);
    virtual ~Fd() = default;
    bool init();
    bool isInit() const;
    bool isSocket() const;
    bool isOpen() const;
    bool isClose() const;
    // bool close();
    void setSysNonBlock(bool is_sys_nonblock);
    void setUserNonBlock(bool is_user_nonblock);
    bool getSysNonBlock() const;
    bool getUserNonBlock() const;
    void setTimeout(const TimeoutType &rs_type, int64_t timeout);
    int64_t getTimeout(const TimeoutType &rs_type) const;

  private:
    int m_fd;
    bool m_is_init;
    bool m_is_socket;
    bool m_is_sys_nonblock;
    bool m_is_user_nonblock;
    bool m_is_closed;
    int64_t m_recv_timeout;
    int64_t m_send_timeout;
    // lon::scheduler::Scheduler *m_ioscheculer;
};

class FdManager
{
  public:
    using MutexType = thread::RWMutex;
    FdManager(int size = 64);
    virtual ~FdManager();

    Fd::Ptr get(int fd, bool auto_create = false);
    void del(int fd);

  private:
    MutexType m_mutex;
    std::vector<Fd::Ptr> m_fds;
    int m_size;
};

#define FDMGR lon::util::Singleton<lon::hook::FdManager>::Instance()

} // namespace hook
} // namespace lon