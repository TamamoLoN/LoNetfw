#include "hook/fd.h"

namespace lon
{
namespace hook
{
hook::Fd::Fd()
    : m_fd(0), m_is_init(false), m_is_socket(false), m_is_sys_nonblock(false),
      m_is_user_nonblock(false), m_is_closed(true), m_recv_timeout(0), m_send_timeout(0)
//   ,m_ioscheculer(nullptr)
{
}

bool hook::Fd::init() {}

bool hook::Fd::isInit() const { return m_is_init; }

bool hook::Fd::isSocket() const { return m_is_socket; }

bool hook::Fd::isOpen() const { return !m_is_closed; }

bool hook::Fd::isClose() const { return m_is_closed; }

bool hook::Fd::close() {}

void hook::Fd::setSysNonBlock(bool is_sys_nonblock) { m_is_sys_nonblock = is_sys_nonblock; }

void hook::Fd::setUserNonBlock(bool is_user_nonblock) { m_is_user_nonblock = is_user_nonblock; }

bool hook::Fd::getSysNonBlock() const { return m_is_sys_nonblock; }

bool hook::Fd::getUserNonBlock() const { return m_is_user_nonblock; }

void hook::Fd::setTimeout(const scheduler::IOScheduler::Event &rs_type, uint64_t timeout)
{
    switch (rs_type)
    {
    case scheduler::IOScheduler::Event::READ:
        m_recv_timeout = timeout;
        break;
    case scheduler::IOScheduler::Event::WRITE:
        m_send_timeout = timeout;
        break;
    default:
        return;
    }
}

uint64_t hook::Fd::getTimeout(const scheduler::IOScheduler::Event &rs_type) const
{
    switch (rs_type)
    {
    case scheduler::IOScheduler::Event::READ:
        return m_recv_timeout;
    case scheduler::IOScheduler::Event::WRITE:
        return m_send_timeout;
    default:
        return 0;
    }
}

} // namespace hook
} // namespace lon