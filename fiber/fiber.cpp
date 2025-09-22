#include "fiber/fiber.h"

namespace lon
{
namespace fiber
{
//主协程分配子协程，子协程执行完毕后控制权回归主协程
static std::atomic<uint64_t> s_fiber_id    = ATOMIC_VAR_INIT(0);
static std::atomic<uint64_t> s_fiber_count = ATOMIC_VAR_INIT(0);

static thread_local Fiber *t_fiber            = nullptr;
static thread_local Fiber::Ptr t_thread_fiber = nullptr; // 主协程
static thread_local Fiber *t_schedule_fiber   = nullptr;

//第一个协程为主协程，实现私有构造
Fiber::Fiber() : m_id(0), m_state(EXEC), m_stack(nullptr)
{
    setThis(this);
    if (getcontext(&m_ctx))
    {
        throw std::runtime_error("getcontext error\n" + util::backtrace(100, 2, "\t"));
    }
    ++s_fiber_count;
    // std::cout << "Fiber create: " << m_id << std::endl;
}

Fiber::Fiber(std::function<void()> cb, size_t stack_size)
    : m_id(++s_fiber_id), m_cb(cb), m_stack_size(stack_size), m_state(INIT), m_stack(nullptr)
{
    m_stack = StackAllocator::allocate(m_stack_size);
    if (getcontext(&m_ctx))
    {
        StackAllocator::deallocate(m_stack);
        throw std::runtime_error("getcontext error\n" + util::backtrace(100, 2, "\t"));
    }
    ++s_fiber_count;
    m_ctx.uc_link          = nullptr;
    m_ctx.uc_stack.ss_sp   = m_stack;
    m_ctx.uc_stack.ss_size = m_stack_size;

    makecontext(&m_ctx, &Fiber::mainFunc, 0);
    // std::cout << "Fiber create: " << m_id << std::endl;
}

Fiber::~Fiber()
{
    --s_fiber_count;
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

        Fiber *cur = t_fiber;
        if (cur == this)
        {
            setThis(nullptr);
        }
    }
    // std::cout << "Fiber destroyed: " << m_id << std::endl;
}

void Fiber::reset(std::function<void()> cb)
{
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
    if (swapcontext(&(t_thread_fiber->m_ctx), &m_ctx))
    {
        throw std::runtime_error("swapcontext error\n" + util::backtrace(100, 2, "\t"));
    }
}

void Fiber::swapIn(Fiber *fiber)
{
    setThis(this);
    t_schedule_fiber = fiber;
    if (m_state == EXEC)
    {
        throw std::runtime_error("swapIn error: m_state is EXEC:[" + stateToString(m_state) +
                                 "]\n" + util::backtrace(100, 2, "\t"));
    }
    m_state = EXEC;
    if (swapcontext(&(fiber->m_ctx), &m_ctx))
    {
        throw std::runtime_error("swapcontext error\n" + util::backtrace(100, 2, "\t"));
    }
}

void Fiber::swapOut()
{
    setThis(t_thread_fiber.get());
    if (swapcontext(&m_ctx, &(t_thread_fiber->m_ctx)))
    {
        throw std::runtime_error("swapcontext error\n" + util::backtrace(100, 2, "\t"));
    }
}

void Fiber::swapOut(Fiber *fiber)
{
    setThis(fiber);
    t_schedule_fiber = nullptr;
    if (swapcontext(&m_ctx, &(fiber->m_ctx)))
    {
        throw std::runtime_error("swapcontext error\n" + util::backtrace(100, 2, "\t"));
    }
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

//静态函数
void Fiber::setThis(Fiber *fiber) { t_fiber = fiber; }

Fiber::Ptr Fiber::getThis()
{
    if (t_fiber)
    {
        return t_fiber->shared_from_this();
    }
    Fiber::Ptr main_fiber(new Fiber);
    if (t_fiber != main_fiber.get())
    {
        throw std::runtime_error("getThis create main_fiber error\n" +
                                 util::backtrace(100, 2, "\t"));
    }
    t_thread_fiber = main_fiber;
    return t_fiber->shared_from_this();
}

void Fiber::yieldToReady()
{
    Fiber::Ptr cur = getThis();
    cur->m_state   = READY;
    cur->swapOut();
}

void Fiber::yieldToReady(Fiber *fiber)
{
    Fiber::Ptr cur = getThis();
    cur->m_state   = READY;
    cur->swapOut(fiber);
}

void Fiber::yieldToHold()
{
    Fiber::Ptr cur = getThis();
    cur->m_state   = HOLD;
    cur->swapOut();
}

void Fiber::yieldToHold(Fiber *fiber)
{
    Fiber::Ptr cur = getThis();
    cur->m_state   = HOLD;
    cur->swapOut(fiber);
}

int64_t Fiber::getFibers() { return s_fiber_count; }

void Fiber::setStateError()
{
    Fiber::Ptr cur = getThis();
    cur->m_state   = ERROR;
}

uint64_t Fiber::getFiberId()
{
    if (t_fiber)
    {
        return t_fiber->getId();
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

    throw std::runtime_error("fiber=" + util::lexical_cast<std::string>(getThis()->m_id) +
                             " can not execute here\n" + util::backtrace(100, 2, "\t"));
}

} // namespace fiber
} // namespace lon