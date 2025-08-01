#pragma once

#include "scheduler/scheduler.h"
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/epoll.h>

namespace lon
{
namespace scheduler
{
class IOScheduler : public Scheduler
{
  public:
    enum Event
    {
        NONE  = 0x00,
        READ  = 0x01, // EPOLLIN
        WRITE = 0x04  // EPOLLOUT
    };

  public:
    using Ptr       = std::shared_ptr<IOScheduler>;
    using MutexType = thread::RWMutex;
    IOScheduler(size_t threads_count = 1, bool use_caller = true, std::string name = "",
                size_t fiber_stack_size = 1024 * 1024);
    ~IOScheduler();

    // 0: success -1: error
    uint8_t addEvent(int fd, Event event, std::function<void()> cb = nullptr);
    bool delEvent(int fd, Event event);
    bool cancelEvent(int fd, Event event);
    bool cancelAll(int fd);

  public:
    static IOScheduler *getThis();

  protected:
    void notify() override;
    bool stopping() override;
    void idle() override;

    void contextResize(size_t size);

  private:
    struct FdContext
    {
        using MutexType = thread::Mutex;
        struct EventContext
        {
            Scheduler *scheduler     = nullptr; //事件执行的scheduler
            fiber::Fiber::Ptr fiber  = nullptr; //事件协程
            std::function<void()> cb = nullptr; //事件回调函数
        };
        EventContext &getContext(Event event);
        void resetContext(EventContext &ctx);
        void triggerEvent(Event event);

        EventContext r_event; //写事件
        EventContext w_event; //读事件
        int fd;               //事件关联句柄
        Event event;          //当前注册事件类型
        FdContext::MutexType mutex;
    };

  private:
    int m_epoll_fd;
    int m_notify_pipe_fd[2];
    std::atomic<size_t> m_waitting_events_count;
    MutexType m_mutex;
    std::vector<FdContext *> m_fd_contexts;
};
} // namespace scheduler
} // namespace lon