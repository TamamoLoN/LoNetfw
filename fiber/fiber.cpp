#include "fiber/fiber.h"

namespace lon
{
namespace fiber
{
// 主协程分配子协程，子协程执行完毕后控制权回归主协程
static std::atomic<uint64_t> s_fiber_id    = ATOMIC_VAR_INIT(0);
static std::atomic<uint64_t> s_fiber_count = ATOMIC_VAR_INIT(0);

static thread_local Fiber *t_cur_fiber      = nullptr; // 当前线程的协程
static thread_local Fiber::Ptr t_main_fiber = nullptr; // 主协程
static thread_local Fiber *t_schedule_fiber = nullptr; // 当前调度期协程

// forward for WinAPI wrapper
#ifdef _WIN32
static VOID CALLBACK FiberProc(LPVOID lpParameter)
{
    Fiber *f = static_cast<Fiber *>(lpParameter);
    // call instance mainFunc
    try
    {
        f->mainFunc();
    }
    // catch (const std::runtime_error &e)
    //{
    //     std::cout << e.what() << std::endl;
    // }
    catch (...)
    {
        // exceptions should already be handled in mainFunc; ensure fiber doesn't fall off
        // std::cout << "unknown error" << std::endl;
    }
    // If mainFunc returns, just exit fiber by switching back to main fiber if any
    // But mainFunc is expected to throw if returns here, similar to your original implementation.
}
#endif

// 第一个协程为主协程，实现私有构造
Fiber::Fiber()
    : m_id(0), m_state(EXEC), m_stack_size(0)
#ifdef _WIN32
      ,
      m_fiber(nullptr)
#else
      ,
      m_stack(nullptr)
#endif
{
    setThis(this);
#ifdef _WIN32
#else
    if (getcontext(&m_ctx))
    {
        throw std::runtime_error("getcontext error\n" + util::backtrace(100, 2, "\t"));
    }
#endif
    ++s_fiber_count;
    // std::cout << "Fiber create: " << m_id << std::endl;
}

Fiber::Fiber(std::function<void()> cb, size_t stack_size)
    : m_id(++s_fiber_id), m_cb(cb), m_stack_size(stack_size), m_state(INIT)
#ifdef _WIN32
      ,
      m_fiber(nullptr)
#else
      ,
      m_stack(nullptr)
#endif
{
#ifdef _WIN32
    // Ensure the thread has a main fiber
    if (!t_main_fiber)
    {
        // Convert thread to fiber if not already
        LPVOID mainFiber = ConvertThreadToFiber(nullptr);
        if (!mainFiber)
        {
            throw std::runtime_error("ConvertThreadToFiber failed");
        }
        // Create a Fiber object to represent the main fiber
        // Note: We create a Fiber via private ctor to indicate it's the main fiber
        Fiber *main_f   = new Fiber(); // will setThis in ctor
        main_f->m_fiber = mainFiber;
        t_main_fiber    = Ptr(main_f);
    }

    // Create a new fiber. CreateFiber accepts stack size; if 0, system decides
    m_fiber = CreateFiber((SIZE_T)m_stack_size, FiberProc, this);
    if (!m_fiber)
    {
        throw std::runtime_error("CreateFiber failed");
    }
#else
    // original ucontext creation...
    m_stack = StackAllocator::allocate(m_stack_size);
    if (getcontext(&m_ctx))
    {
        StackAllocator::deallocate(m_stack);
        throw std::runtime_error("getcontext error\n" + util::backtrace(100, 2, "\t"));
    }

    m_ctx.uc_link          = nullptr;
    m_ctx.uc_stack.ss_sp   = m_stack;
    m_ctx.uc_stack.ss_size = m_stack_size;

    makecontext(&m_ctx, &Fiber::mainFunc, 0);
    // std::cout << "Fiber create: " << m_id << std::endl;
#endif
    ++s_fiber_count;
}

Fiber::~Fiber()
{
    --s_fiber_count;
#ifdef _WIN32
    // windows不能delete主协程
    if (m_fiber && this != t_main_fiber.get())
    {
        DeleteFiber(m_fiber);
        m_fiber = nullptr;
        if (m_state != TERM && m_state != INIT && m_state == ERROR)
        {
            throw std::runtime_error("m_state error: m_state not TERM or INIT:[" +
                                     stateToString(m_state) + "]\n" +
                                     util::backtrace(100, 2, "\t"));
        }
    }
#else
    if (m_stack)
    {
        StackAllocator::deallocate(m_stack);
        if (m_state != TERM && m_state != INIT && m_state == ERROR)
        {
            throw std::runtime_error("m_state error: m_state not TERM or INIT:[" +
                                     stateToString(m_state) + "]\n" +
                                     util::backtrace(100, 2, "\t"));
        }
    }
#endif
    else
    {
        if (m_cb)
        {
            throw std::runtime_error("");
        }
        if (m_state != EXEC)
        {
            throw std::runtime_error("");
        }

        Fiber *cur = t_cur_fiber;
        if (cur == this)
        {
            setThis(nullptr);
        }
    }
    // std::cout << "Fiber destroyed: " << m_id << std::endl;
}

void Fiber::reset(std::function<void()> cb)
{
#ifdef _WIN32
    if (this == t_main_fiber.get())
    {
        throw std::runtime_error("cannot reset main fiber " + util::backtrace(100, 2, "\t"));
    }
    if (m_state != TERM && m_state != INIT && m_state == ERROR)
    {
        throw std::runtime_error("reset error: m_state not TERM or INIT:[" +
                                 stateToString(m_state) + "]\n" + util::backtrace(100, 2, "\t"));
    }
    m_cb = std::move(cb);
    if (m_fiber)
    {
        DeleteFiber(m_fiber);
        m_fiber = nullptr;
    }
    m_fiber = CreateFiber((SIZE_T)m_stack_size, FiberProc, this);
    if (!m_fiber)
    {
        throw std::runtime_error("CreateFiber error\n" + util::backtrace(100, 2, "\t"));
    }

    m_state = INIT;
#else
    if (m_stack == nullptr)
    {
        throw std::runtime_error("reset error: m_stack is null\n" + util::backtrace(100, 2, "\t"));
    }
    if (m_state != TERM && m_state != INIT && m_state == ERROR)
    {
        throw std::runtime_error("reset error: m_state not TERM or INIT:[" +
                                 stateToString(m_state) + "]\n" + util::backtrace(100, 2, "\t"));
    }
    m_cb = cb;
    if (getcontext(&m_ctx))
    {
        StackAllocator::deallocate(m_stack);
        throw std::runtime_error("getcontext error\n" + util::backtrace(100, 2, "\t"));
    }
    m_ctx.uc_link          = nullptr;
    m_ctx.uc_stack.ss_sp   = m_stack;
    m_ctx.uc_stack.ss_size = m_stack_size;

    makecontext(&m_ctx, &Fiber::mainFunc, 0);
    m_state = INIT;
#endif
}

void Fiber::swapIn()
{
    setThis(this);
    if (m_state == EXEC)
    {
        throw std::runtime_error("swapIn error: m_state is EXEC:[" + stateToString(m_state) +
                                 "]\n" + util::backtrace(100, 2, "\t"));
    }
    m_state = EXEC;
#ifdef _WIN32
    // Switch to this fiber. current fiber pointer must be set by ConvertThreadToFiber or prior
    // Switch
    SwitchToFiber(m_fiber);
#else
    if (swapcontext(&(t_main_fiber->m_ctx), &m_ctx))
    {
        throw std::runtime_error("swapcontext error\n" + util::backtrace(100, 2, "\t"));
    }
#endif
}

void Fiber::swapIn(Fiber *main_fiber)
{
    setThis(this);
    t_schedule_fiber = main_fiber;
    if (m_state == EXEC)
    {
        throw std::runtime_error("swapIn error: m_state is EXEC:[" + stateToString(m_state) +
                                 "]\n" + util::backtrace(100, 2, "\t"));
    }
    m_state = EXEC;
#ifdef _WIN32
    // Switch from schedule fiber to this fiber
    SwitchToFiber(m_fiber);
#else
    if (swapcontext(&(main_fiber->m_ctx), &m_ctx))
    {
        throw std::runtime_error("swapcontext error\n" + util::backtrace(100, 2, "\t"));
    }
#endif
}

void Fiber::swapOut()
{
    setThis(t_main_fiber.get());
#ifdef _WIN32
    // switch back to thread's main fiber
    if (t_main_fiber && t_main_fiber->m_fiber)
    {
        SwitchToFiber(t_main_fiber->m_fiber);
    }
    else
    {
        throw std::runtime_error("swapcontext error\n" + util::backtrace(100, 2, "\t"));
    }
#else
    if (swapcontext(&m_ctx, &(t_main_fiber->m_ctx)))
    {
        throw std::runtime_error("swapcontext error\n" + util::backtrace(100, 2, "\t"));
    }
#endif
}

void Fiber::swapOut(Fiber *main_fiber)
{
    setThis(main_fiber);
    t_schedule_fiber = nullptr;
#ifdef _WIN32
    if (main_fiber && main_fiber->m_fiber)
    {
        SwitchToFiber(main_fiber->m_fiber);
    }
    else
    {
        throw std::runtime_error("swapcontext error\n" + util::backtrace(100, 2, "\t"));
    }
#else
    if (swapcontext(&m_ctx, &(main_fiber->m_ctx)))
    {
        throw std::runtime_error("swapcontext error\n" + util::backtrace(100, 2, "\t"));
    }
#endif
}

void Fiber::setState(Fiber::State state) { m_state = state; }

Fiber::State Fiber::getState() const { return m_state; }

uint64_t Fiber::getId() const { return m_id; }

std::string Fiber::stateToString(State state) const
{
    switch (state)
    {
#define XX(str)                                                                                    \
    case str:                                                                                      \
        return #str;
        XX(ERROR)
        XX(INIT)
        XX(HOLD)
        XX(EXEC)
        XX(TERM)
        XX(READY)
#undef XX
    default:
        return "UNKNOWN";
    }
}

// 静态函数
void Fiber::setThis(Fiber *fiber) { t_cur_fiber = fiber; }

Fiber::Ptr Fiber::getThis()
{
    if (t_cur_fiber)
    {
        return t_cur_fiber->shared_from_this();
    }
#ifdef _WIN32
    // create main fiber object if missing
    // ConvertThreadToFiber must be called before CreateFiber; ensure main fiber exists
    LPVOID mainFiber = ConvertThreadToFiber(nullptr);
    if (!mainFiber)
    {
        throw std::runtime_error("ConvertThreadToFiber failed in getThis");
    }
    Fiber *main_f   = new Fiber(); // uses private ctor -> sets this
    main_f->m_fiber = mainFiber;
    t_main_fiber    = Ptr(main_f);
#else
    Fiber::Ptr main_fiber(new Fiber);
    if (t_cur_fiber != main_fiber.get())
    {
        throw std::runtime_error("getThis create main_fiber error\n" +
                                 util::backtrace(100, 2, "\t"));
    }
    t_main_fiber = main_fiber;
#endif
    return t_cur_fiber->shared_from_this();
}

void Fiber::yieldToReady()
{
    Fiber::Ptr cur = getThis();
    cur->m_state   = READY;
    cur->swapOut();
}

void Fiber::yieldToReady(Fiber *main_fiber)
{
    Fiber::Ptr cur = getThis();
    cur->m_state   = READY;
    cur->swapOut(main_fiber);
}

void Fiber::yieldToHold()
{
    Fiber::Ptr cur = getThis();
    cur->m_state   = HOLD;
    cur->swapOut();
}

void Fiber::yieldToHold(Fiber *main_fiber)
{
    Fiber::Ptr cur = getThis();
    cur->m_state   = HOLD;
    cur->swapOut(main_fiber);
}

int64_t Fiber::getFibers() { return s_fiber_count; }

void Fiber::setStateError()
{
    Fiber::Ptr cur = getThis();
    cur->m_state   = ERROR;
}

uint64_t Fiber::getFiberId()
{
    if (t_cur_fiber)
    {
        return t_cur_fiber->getId();
    }
    return 0;
}

void Fiber::mainFunc()
{
    Fiber::Ptr &&cur = std::move(getThis());
    // Fiber::Ptr cur = getThis();
    try
    {
        cur->m_cb();
        cur->m_cb    = nullptr;
        cur->m_state = TERM;
    }
    catch (std::exception &ex)
    {
        std::cout << "fiber=" << cur->m_id << " error:" << ex.what() << std::endl;
        cur->m_state = ERROR;
    }
    catch (...)
    {
        cur->m_state = ERROR;
    }
    // auto cur_raw = cur.get();
    // cur.reset();
    // //函数执行完毕后需要手动切回主协程
    // cur_raw->swapOut();
    if (t_schedule_fiber != nullptr)
    {
        cur->swapOut(t_schedule_fiber);
    }
    else
    {
        cur->swapOut();
    }
#ifdef _WIN32
    return;
#else
    throw std::runtime_error("fiber=" + util::lexical_cast<std::string>(getThis()->m_id) +
                             " can not execute here\n" + util::backtrace(100, 2, "\t"));
#endif
}

} // namespace fiber
} // namespace lon