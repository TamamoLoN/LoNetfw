#pragma once

// #include "thread/thread.h"
#include "util/util.h"
#include <atomic>
#include <memory>
#include <ucontext.h>

namespace lon
{
namespace fiber
{
class Fiber : public std::enable_shared_from_this<Fiber>
{
  public:
    using Ptr            = std::shared_ptr<Fiber>;
    using StackAllocator = util::Allocator;

    explicit Fiber(std::function<void()> cb, size_t stack_size = 0);
    virtual ~Fiber();

    enum State
    {
#ifdef _WIN32
        ERROR_ = -1,
#else
        ERROR = -1,
#endif
        INIT = 0,
        HOLD,
        EXEC,
        TERM,
        READY
    };

    // 重置协程函数，并重置状态 INIT or TERM
    void reset(std::function<void()> cb);

    // 切换到当前协程执行
    void swapIn();
    void swapIn(Fiber *main_fiber);

    // 把当前协程切换至后台执行
    void swapOut();
    void swapOut(Fiber *main_fiber);

    void setState(State state);
    State getState() const;
    uint64_t getId() const;
    std::string stateToString(State state) const;

    // 设置当前协程
    static void setThis(Fiber *fiber);

    // 返回当前协程
    static Ptr getThis();

    // 协程切换到后台，并设置为Ready状态
    static void yieldToReady();
    static void yieldToReady(Fiber *main_fiber);

    // 协程切换到后台，并设置为Hold状态
    static void yieldToHold();
    static void yieldToHold(Fiber *main_fiber);

    // 获取总协程数
    static int64_t getFibers();

    static void setStateError();

    static uint64_t getFiberId();

    static void mainFunc();

  private:
    Fiber();

  private:
    uint64_t m_id;
    size_t m_stack_size;
    State m_state;
#ifdef _WIN32
    void *m_fiber; // LPVOID
    // note: we will not manage m_stack pointer here because CreateFiber allocates it.
#else
    ucontext_t m_ctx;
    void *m_stack;
#endif
    std::function<void()> m_cb;
};
} // namespace fiber
} // namespace lon