#pragma once

#include "scheduler/scheduler.h"
#include "scheduler/timer.h"
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#ifdef _WIN32
#include <wepoll.h>
#else
#include <sys/epoll.h>
#endif

namespace lon
{
namespace scheduler
{
#ifdef _WIN32
static int pipe(SOCKET sv[2]);
#endif
class IOScheduler : public Scheduler, public TimerManager
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
    int8_t addEvent(int fd, Event event, std::function<void()> cb = nullptr);
    bool delEvent(int fd, Event event);
    bool cancelEvent(int fd, Event event);
    bool cancelAll(int fd);

  public:
    static IOScheduler *getThis();

  protected:
    void notify() override;
    bool stopping() override;
    void idle() override;
    void onTimerInsertAtFront() override;

    bool stopping(uint64_t &timeout);
    void contextResize(size_t size);

  private:
    struct FdContext
    {
        using MutexType = thread::Mutex;
        struct EventContext
        {
            Scheduler *scheduler     = nullptr; // 事件执行的scheduler
            fiber::Fiber::Ptr fiber  = nullptr; // 事件协程
            std::function<void()> cb = nullptr; // 事件回调函数
        };
        EventContext &getContext(Event event);
        void resetContext(EventContext &ctx);
        void triggerEvent(Event event);

        EventContext r_event;      // 写事件
        EventContext w_event;      // 读事件
        int fd;                    // 事件关联句柄
        Event event = Event::NONE; // 当前注册事件类型
        FdContext::MutexType mutex;
    };

  private:
#ifdef _WIN32
    HANDLE m_epoll_fd;
    SOCKET m_notify_pipe_fd[2];
#else
    // epoll文件句柄
    int m_epoll_fd;
    // pipe文件句柄，其中[0]表示读端，[1]表示写端
    int m_notify_pipe_fd[2];
#endif
    // 等待执行的事件数量
    std::atomic<size_t> m_waitting_events_count;
    MutexType m_mutex;
    std::vector<FdContext *> m_fd_contexts;
};
} // namespace scheduler
} // namespace lon