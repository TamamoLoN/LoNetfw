#pragma once

// likely/unlikely 宏，用于分支预测优化
#if defined __GNUC__ || defined __llvm__
#define LON_LIKELY(x) __builtin_expect((x), 1)
#define LON_UNLIKELY(x) __builtin_expect((x), 0)
#else
#define LON_LIKELY(x) (x)
#define LON_UNLIKELY(x) (x)
#endif

#define LONETFW_VERSION "1.0.0"

// XX(scheme, default_port)
#define PROTOCAL_MAP(XX)                                                                           \
    XX(http, 80)                                                                                   \
    XX(https, 443)                                                                                 \
    XX(rtsp, 554)                                                                                  \
    XX(mqtt, 1883)
