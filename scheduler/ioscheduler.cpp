#include "scheduler/ioscheduler.h"

namespace lon
{
namespace scheduler
{
static auto g_logger = LON_LOG_ROOT;

#ifdef _WIN32
static int pipe(SOCKET sv[2])
{
    SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == INVALID_SOCKET)
        return -1;

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port        = 0;

    if (bind(listener, (sockaddr *)&addr, sizeof(addr)) != 0)
        return -1;

    if (listen(listener, 1) != 0)
        return -1;

    int len = sizeof(addr);
    getsockname(listener, (sockaddr *)&addr, &len);

    sv[0] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connect(sv[0], (sockaddr *)&addr, sizeof(addr)) != 0)
        return -1;

    sv[1] = accept(listener, nullptr, nullptr);
    closesocket(listener);

    return (sv[1] != INVALID_SOCKET) ? 0 : -1;
}

#endif

IOScheduler::IOScheduler(size_t threads_count, bool use_caller, std::string name,
                         size_t fiber_stack_size)
    : Scheduler(threads_count, use_caller, name, fiber_stack_size), TimerManager(),
      m_waitting_events_count({0}), m_epoll_fd(0)
{
    memset(m_notify_pipe_fd, 0, sizeof(m_notify_pipe_fd));

#ifdef _WIN32
    m_epoll_fd = epoll_create(5000);
    LON_ASSERT(m_epoll_fd != nullptr);

    int rt = pipe(m_notify_pipe_fd);
    LON_ASSERT(!rt);
    epoll_event event;
    memset(&event, 0, sizeof(event));
    event.events    = EPOLLIN | EPOLLONESHOT;
    event.data.sock = m_notify_pipe_fd[0];

    u_long nb = 1;
    rt        = ioctlsocket(m_notify_pipe_fd[0], FIONBIO, &nb);
    LON_ASSERT(!rt);

    rt = epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_notify_pipe_fd[0], &event);
    LON_ASSERT(!rt);
#else
    m_epoll_fd = epoll_create(5000);
    LON_ASSERT(m_epoll_fd != -1);

    int rt = pipe(m_notify_pipe_fd);
    LON_ASSERT(!rt);

    epoll_event event;
    memset(&event, 0, sizeof(event));
    event.events  = EPOLLIN | EPOLLET;
    event.data.fd = m_notify_pipe_fd[0];

    rt = fcntl(m_notify_pipe_fd[0], F_SETFL, O_NONBLOCK);
    LON_ASSERT(!rt);

    rt = epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_notify_pipe_fd[0], &event);
    LON_ASSERT(!rt);
#endif

    contextResize(32);

    start();
}

IOScheduler::~IOScheduler()
{
    stop();
#ifdef _WIN32
    epoll_close(m_epoll_fd);
    closesocket(m_notify_pipe_fd[0]);
    closesocket(m_notify_pipe_fd[1]);
#else
    close(m_epoll_fd);
    close(m_notify_pipe_fd[0]);
    close(m_notify_pipe_fd[1]);
#endif
    for (auto &it : m_fd_contexts)
    {
        delete it;
        it = nullptr;
    }
}

int8_t IOScheduler::addEvent(int fd, Event event, std::function<void()> cb)
{
    // 保证fd即是索引
    FdContext *fd_ctx = nullptr;
    MutexType::RdLock rlock(m_mutex);
    if ((int)m_fd_contexts.size() > fd)
    {
        fd_ctx = m_fd_contexts[fd];
        rlock.unlock();
    }
    else
    {
        rlock.unlock();
        MutexType::WrLock wlock(m_mutex);
        contextResize(fd * 1.5);
        fd_ctx = m_fd_contexts[fd];
    }
    FdContext::MutexType::Lock fd_ctx_lock(fd_ctx->mutex);
    // 一个句柄一般不会重复加同一个事件， 可能是两个不同的线程在操控同一个句柄添加事件
    if (fd_ctx->event & event)
    {
        LON_ERROR(g_logger) << "can not add same event ,fd=" << fd << " event=" << event
                            << " fd_ctx->event=" << fd_ctx->event;
        return -1;
    }

    int op = fd_ctx->event ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
    epoll_event epevent;
    memset(&epevent, 0, sizeof(epevent));
#ifdef _WIN32
    epevent.events = EPOLLONESHOT | fd_ctx->event | event;
#else
    epevent.events = EPOLLET | fd_ctx->event | event;
#endif
    epevent.data.ptr = fd_ctx;
    int ret          = epoll_ctl(m_epoll_fd, op, fd, &epevent);
    if (ret != 0)
    {
        LON_ERROR(g_logger) << "epoll_ctl error ,m_epoll_fd=" << m_epoll_fd << " op=" << op
                            << " fd=" << fd << " epevent.events=" << epevent.events
                            << " errno=" << errno << ": " << strerror(errno);
        return -1;
    }
    ++m_waitting_events_count;
    fd_ctx->event   = (Event)(fd_ctx->event | event);
    auto &event_ctx = fd_ctx->getContext(event);
    LON_ASSERT(!event_ctx.cb && !event_ctx.fiber && !event_ctx.scheduler);
    event_ctx.scheduler = Scheduler::getThis();
    if (cb)
    {
        event_ctx.cb.swap(cb);
    }
    else
    {
        event_ctx.fiber = fiber::Fiber::getThis();
        LON_ASSERT(event_ctx.fiber->getState() == fiber::Fiber::EXEC);
    }

    return 0;
}

bool IOScheduler::delEvent(int fd, Event event)
{
    MutexType::RdLock rlock(m_mutex);
    if ((int)m_fd_contexts.size() <= fd)
    {
        return false;
    }
    FdContext *fd_ctx = m_fd_contexts[fd];
    rlock.unlock();

    FdContext::MutexType::Lock fd_ctx_lock(fd_ctx->mutex);
    if (!(fd_ctx->event & event))
    {
        return false;
    }
    Event new_event = (Event)(fd_ctx->event & ~event);
    int op          = new_event ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epevent;
    memset(&epevent, 0, sizeof(epevent));
#ifdef _WIN32
    epevent.events = EPOLLONESHOT | new_event;
#else
    epevent.events = EPOLLET | new_event;
#endif
    epevent.data.ptr = fd_ctx;
    int ret          = epoll_ctl(m_epoll_fd, op, fd, &epevent);
    if (ret != 0)
    {
        LON_ERROR(g_logger) << "epoll_ctl error ,m_epoll_fd=" << m_epoll_fd << " op=" << op
                            << " fd=" << fd << " epevent.events=" << epevent.events
                            << " errno=" << errno << ": " << strerror(errno);
        return false;
    }
    --m_waitting_events_count;
    fd_ctx->event   = new_event;
    auto &event_ctx = fd_ctx->getContext(event);
    fd_ctx->resetContext(event_ctx);

    return true;
}

bool IOScheduler::cancelEvent(int fd, Event event)
{
    MutexType::RdLock rlock(m_mutex);
    if ((int)m_fd_contexts.size() <= fd)
    {
        return false;
    }
    FdContext *fd_ctx = m_fd_contexts[fd];
    rlock.unlock();

    FdContext::MutexType::Lock fd_ctx_lock(fd_ctx->mutex);
    if (!(fd_ctx->event & event))
    {
        return false;
    }
    Event new_event = (Event)(fd_ctx->event & ~event);
    int op          = new_event ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epevent;
    memset(&epevent, 0, sizeof(epevent));
#ifdef _WIN32
    epevent.events = EPOLLONESHOT | new_event;
#else
    epevent.events = EPOLLET | new_event;
#endif
    epevent.data.ptr = fd_ctx;
    int ret          = epoll_ctl(m_epoll_fd, op, fd, &epevent);
    if (ret != 0)
    {
        LON_ERROR(g_logger) << "epoll_ctl error ,m_epoll_fd=" << m_epoll_fd << " op=" << op
                            << " fd=" << fd << " epevent.events=" << epevent.events
                            << " errno=" << errno << ": " << strerror(errno);
        return false;
    }
    fd_ctx->triggerEvent(event);
    --m_waitting_events_count;

    return true;
}

bool IOScheduler::cancelAll(int fd)
{
    MutexType::RdLock rlock(m_mutex);
    if ((int)m_fd_contexts.size() <= fd)
    {
        return false;
    }
    FdContext *fd_ctx = m_fd_contexts[fd];
    rlock.unlock();

    FdContext::MutexType::Lock fd_ctx_lock(fd_ctx->mutex);
    if (!fd_ctx->event)
    {
        return false;
    }
    int op = EPOLL_CTL_DEL;
    epoll_event epevent;
    memset(&epevent, 0, sizeof(epevent));
    epevent.events   = 0;
    epevent.data.ptr = fd_ctx;
    int ret          = epoll_ctl(m_epoll_fd, op, fd, &epevent);
    if (ret != 0)
    {
        LON_ERROR(g_logger) << "epoll_ctl error ,m_epoll_fd=" << m_epoll_fd << " op=" << op
                            << " fd=" << fd << " epevent.events=" << epevent.events
                            << " errno=" << errno << ": " << strerror(errno);
        return false;
    }
    if (fd_ctx->event & Event::READ)
    {
        fd_ctx->triggerEvent(Event::READ);
        --m_waitting_events_count;
    }
    if (fd_ctx->event & Event::WRITE)
    {
        fd_ctx->triggerEvent(Event::WRITE);
        --m_waitting_events_count;
    }
    LON_ASSERT(fd_ctx->event == Event::NONE);

    return true;
} // namespace scheduler

void IOScheduler::notify()
{
    if (!hasIdleThreads())
    {
        return;
    }
    LON_DEBUG(g_logger) << "notify";
#ifdef _WIN32
    char c  = 'T';
    int ret = send(m_notify_pipe_fd[1], &c, 1, 0);
    LON_ASSERT(ret == 1);
#else
    int ret        = write(m_notify_pipe_fd[1], "T", 1);
#endif
    LON_ASSERT(ret == 1);
}

bool IOScheduler::stopping()
{
    uint64_t timeout = 0;
    return stopping(timeout);
}

bool IOScheduler::stopping(uint64_t &timeout)
{
    timeout = getNextTimerTimeMs();
    return timeout == ~0ull && m_waitting_events_count == 0 && Scheduler::stopping();
}

void IOScheduler::idle()
{
    LON_DEBUG(g_logger) << "idle";
    epoll_event *events = new epoll_event[64]();
    std::shared_ptr<epoll_event> shared_events(events, [](epoll_event *ptr) {
        delete[] ptr;
        ptr = nullptr;
    });

    while (true)
    {
        uint64_t next_timeout = 0;
        if (stopping(next_timeout))
        {
            LON_INFO(g_logger) << "ioscheduler[" << getName() << "] idle stopping exit";
            break;
        }

        int ret = 0;
        do
        {
            static const int MAX_TIMEOUT = 3000; // ms
            if (next_timeout != ~0ull)
            {
                next_timeout = (int)next_timeout > MAX_TIMEOUT ? MAX_TIMEOUT : (int)next_timeout;
            }
            else
            {
                next_timeout = MAX_TIMEOUT;
            }
            ret = epoll_wait(m_epoll_fd, events, 64, (int)next_timeout);
            if (ret < 0 && errno == EINTR)
            {
            }
            else
            {
                break;
            }
        } while (true);

        std::vector<std::function<void()>> cbs = {};
        getExpiredCbsList(cbs);
        if (!cbs.empty())
        {
            schedule(cbs.begin(), cbs.end());
            cbs.clear();
        }

        for (int cnt = 0; cnt < ret; ++cnt)
        {
            epoll_event &event = events[cnt];
#ifdef _WIN32
            if (event.data.sock == m_notify_pipe_fd[0])
            {
                char msg = 0;
                while (recv(m_notify_pipe_fd[0], &msg, 1, 0) == 1)
                {
                }
                continue;
            }
#else
            if (event.data.fd == m_notify_pipe_fd[0])
            {
                uint8_t msg = 0;
                while (read(m_notify_pipe_fd[0], &msg, 1) == 1)
                {
                }
                continue;
            }
#endif
            FdContext *fd_ctx = (FdContext *)event.data.ptr;
            FdContext::MutexType::Lock fd_ctx_lock(fd_ctx->mutex);
            if (event.events & (EPOLLERR | EPOLLHUP))
            {
                event.events |= (EPOLLIN | EPOLLOUT) & fd_ctx->event;
            }
            int real_event = Event::NONE;
            if (event.events & EPOLLIN)
            {
                real_event |= Event::READ;
            }
            if (event.events & EPOLLOUT)
            {
                real_event |= Event::WRITE;
            }

            if ((fd_ctx->event & real_event) == Event::NONE)
            {
                continue;
            }
            int left_event = (fd_ctx->event & ~real_event);
            int op         = left_event ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
#ifdef _WIN32
            event.events = EPOLLONESHOT | left_event;
#else
            event.events = EPOLLET | left_event;
#endif

            int ret_epoll_ctl = epoll_ctl(m_epoll_fd, op, fd_ctx->fd, &event);
            if (ret_epoll_ctl)
            {
                LON_ERROR(g_logger)
                    << "epoll_ctl error ,m_epoll_fd=" << m_epoll_fd << " op=" << op
                    << " fd_ctx->fd=" << fd_ctx->fd << " event.events=" << event.events
                    << " errno=" << errno << ": " << strerror(errno);
                continue;
            }

            if (real_event & Event::READ)
            {
                fd_ctx->triggerEvent(Event::READ);
                --m_waitting_events_count;
            }
            if (real_event & Event::WRITE)
            {
                fd_ctx->triggerEvent(Event::WRITE);
                --m_waitting_events_count;
            }
        }
        // 减少t_fiber引用计数，防止idle fiber退出时，持有太多t_fiber的引用
        auto &&cur = std::move(fiber::Fiber::getThis());
        cur->swapOut(Scheduler::getMainFiber());
        // fiber::Fiber::yieldToHold(Scheduler::getMainFiber());
        // LON_WARN(g_logger) << fiber::Fiber::getThis()->getFiberId();
    }
}

void IOScheduler::onTimerInsertAtFront() { notify(); }

void IOScheduler::contextResize(size_t size)
{
    m_fd_contexts.resize(size);
    for (size_t cnt = 0; cnt < m_fd_contexts.size(); ++cnt)
    {
        if (!m_fd_contexts[cnt])
        {
            m_fd_contexts[cnt]     = new FdContext;
            m_fd_contexts[cnt]->fd = cnt;
        }
    }
}

// 静态方法
IOScheduler *IOScheduler::getThis() { return dynamic_cast<IOScheduler *>(Scheduler::getThis()); }

IOScheduler::FdContext::EventContext &IOScheduler::FdContext::getContext(Event event)
{
    switch (event)
    {
    case Event::READ:
        return r_event;
        break;
    case Event::WRITE:
        return w_event;
        break;
    default:
        LON_ASSERT_(false, "get context error");
        break;
    }
}

void IOScheduler::FdContext::resetContext(EventContext &ctx)
{
    ctx.cb = nullptr;
    ctx.fiber.reset();
    ctx.scheduler = nullptr;
}

void IOScheduler::FdContext::triggerEvent(Event event)
{
    LON_ASSERT(event & this->event);
    this->event = (Event)((~event) & this->event);
    auto &ctx   = getContext(event);
    if (ctx.cb)
    {
        ctx.scheduler->schedule(&ctx.cb);
    }
    else if (ctx.fiber)
    {
        ctx.scheduler->schedule(&ctx.fiber);
    }
    else
    {
        LON_ASSERT_(false, "error ctx");
    }
    ctx.scheduler = nullptr;
}

} // namespace scheduler
} // namespace lon