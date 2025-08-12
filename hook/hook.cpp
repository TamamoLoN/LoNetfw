#include "hook/hook.h"

namespace lon
{
namespace hook
{

#define HOOK_FUN(XX)                                                                               \
    XX(sleep)                                                                                      \
    XX(usleep)

Hook::Hook()
{
    static int is_inited = false;
    if (is_inited)
    {
        return;
    }
#define XX(name) name##_f = (name##_fun)dlsym(RTLD_NEXT, #name);
    HOOK_FUN(XX)
#undef XX
}

Hook::~Hook() {}

Hook &Hook::Instance()
{
    static Hook instance;
    return instance;
}

void Hook::enable() { util::HookState::enable(); }

void Hook::disable() { util::HookState::disable(); }

auto s_hook = Hook::Instance();

} // namespace hook
} // namespace lon
extern "C"
{
#define XX(name) name##_fun name##_f = nullptr;
    HOOK_FUN(XX)
#undef XX

    unsigned int sleep(unsigned int seconds)
    {
        if (!lon::util::HookState::isEnable())
        {
            return sleep_f(seconds);
        }
        auto fiber = lon::fiber::Fiber::getThis();
        auto ios   = lon::scheduler::IOScheduler::getThis();
        ios->addTimer(
            seconds * 1000, [fiber, ios]() { ios->schedule(fiber); }, false);
        lon::fiber::Fiber::yieldToHold(lon::scheduler::IOScheduler::getMainFiber());

        return 0;
    }

    int usleep(useconds_t usec)
    {
        if (!lon::util::HookState::isEnable())
        {
            return usleep_f(usec);
        }
        auto fiber = lon::fiber::Fiber::getThis();
        auto ios   = lon::scheduler::IOScheduler::getThis();
        ios->addTimer(
            usec / 1000, [fiber, ios]() { ios->schedule(fiber); }, false);
        lon::fiber::Fiber::yieldToHold(lon::scheduler::IOScheduler::getMainFiber());

        return 0;
    }
}
