#pragma once

// likely/unlikely 宏，用于分支预测优化
#if defined __GNUC__ || defined __llvm__
#define LON_LIKELY(x) __builtin_expect((x), 1)
#define LON_UNLIKELY(x) __builtin_expect((x), 0)
#else
#define LON_LIKELY(x) (x)
#define LON_UNLIKELY(x) (x)
#endif