#include "fiber/fiber.h"

namespace lon
{
namespace fiber
{
static std::atomic<uint64_t> s_fiber_id    = ATOMIC_VAR_INIT(0);
static std::atomic<uint64_t> s_fiber_count = ATOMIC_VAR_INIT(0);

static thread_local Fiber *t_fiber                             = nullptr;
static thread_local std::shared_ptr<Fiber::Ptr> t_thread_fiber = nullptr; // main协程

Fiber::Fiber(std::function<void()> cb, size_t stack_size)
    : m_id(0), m_cb(cb), m_stack_size(stack_size), m_state(INIT), m_stack(nullptr)
{
}

Fiber::~Fiber() {}

void Fiber::reset(std::function<void()> cb) {}

void Fiber::swapIn() {}

void Fiber::swapOut() {}

Fiber::State Fiber::getState() const {}

//静态函数
Fiber::Ptr Fiber::getThis() {}

void Fiber::yieldToReady() {}

void Fiber::yieldToHold() {}

int64_t Fiber::getFibers() {}

void Fiber::mainFunc() {}

} // namespace fiber
} // namespace lon