#pragma once
#include "scheduler/ioscheduler.h"
namespace lon
{
namespace hook
{
class Fd : public std::enable_shared_from_this<Fd>
{
  public:
    using Ptr = std::shared_ptr<Fd>;
    Fd();
    virtual ~Fd() = default;
    bool init();
    bool isInit() const;
    bool isSocket() const;
    bool isOpen() const;
    bool isClose() const;
    bool close();
    void setSysNonBlock(bool is_sys_nonblock);
    void setUserNonBlock(bool is_user_nonblock);
    bool getSysNonBlock() const;
    bool getUserNonBlock() const;
    void setTimeout(const scheduler::IOScheduler::Event &rs_type, uint64_t timeout);
    uint64_t getTimeout(const scheduler::IOScheduler::Event &rs_type) const;

  private:
    int m_fd;
    bool m_is_init;
    bool m_is_socket;
    bool m_is_sys_nonblock;
    bool m_is_user_nonblock;
    bool m_is_closed;
    uint64_t m_recv_timeout;
    uint64_t m_send_timeout;
    // lon::scheduler::Scheduler *m_ioscheculer;
};

class FdManager
{
  public:
    using MutexType = thread::RWMutex;
    FdManager();
    virtual ~FdManager();

    Fd::Ptr get(int fd, bool auto_create = false);

  private:
    MutexType m_mutex;
    std::vector<Fd::Ptr> m_fds;
};

} // namespace hook
} // namespace lon