#pragma once

#include "scheduler/ioscheduler.h"
#include "util/util.h"
#include <dlfcn.h>
#include <unistd.h>

namespace lon
{
namespace hook
{
class Hook
{
  public:
    Hook();
    ~Hook();
    static Hook &Instance();
    static void enable();
    static void disable();
};
} // namespace hook
} // namespace lon

extern "C"
{
    typedef unsigned int (*sleep_fun)(unsigned int seconds);
    extern sleep_fun sleep_f;

    typedef int (*usleep_fun)(useconds_t usec);
    extern usleep_fun usleep_f;
}
