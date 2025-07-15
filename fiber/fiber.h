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
        ERROR = -1,
        INIT  = 0,
        HOLD,
        EXEC,
        TERM,
        READY
    };

    //重置协程函数，并重置状态 INIT or TERM
    void reset(std::function<void()> cb);

    //切换到当前协程执行
    void swapIn();

    //把当前协程切换至后台执行
    void swapOut();
    State getState() const;
    uint64_t getId() const;

    //设置当前协程
    static void setThis(Fiber *fiber);

    //返回当前协程
    static Ptr getThis();

    //协程切换到后台，并设置为Ready状态
    static void yieldToReady();

    //协程切换到后台，并设置为Hold状态
    static void yieldToHold();

    //获取总协程数
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
    ucontext_t m_ctx;
    void *m_stack;
    std::function<void()> m_cb;
};
} // namespace fiber
} // namespace lon