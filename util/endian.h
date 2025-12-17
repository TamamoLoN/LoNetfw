#pragma once

#ifdef _WIN32
#include <stdlib.h>
#define bswap_16 _byteswap_ushort
#define bswap_32 _byteswap_ulong
#define bswap_64 _byteswap_uint64
#else
#include <byteswap.h>
#endif
#include <iostream>
#include <stdint.h>

#define LON_LITTLE_ENDIAN 1
#define LON_BIG_ENDIAN 2

namespace lon
{
namespace util
{
template <typename T>
typename std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type byteswap(T value)
{
    return (T)bswap_64((uint64_t)value);
}

template <typename T>
typename std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type byteswap(T value)
{
    return (T)bswap_32((uint32_t)value);
}

template <typename T>
typename std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type byteswap(T value)
{
    return (T)bswap_16((uint16_t)value);
}

#ifdef _WIN32
// windows只有小端序
#define LON_ENDIAN LON_LITTLE_ENDIAN
#else
#if BYTE_ORDER == BIG_ENDIAN
#define LON_ENDIAN LON_BIG_ENDIAN
#else
#define LON_ENDIAN LON_LITTLE_ENDIAN
#endif
#endif

#if LON_ENDIAN == LON_LITTLE_ENDIAN
template <typename T> T byteswapToBigEndian(T t) { return byteswap(t); }
template <typename T> T byteswapToLittleEndian(T t) { return t; }
#else
template <typename T> T byteswapToBigEndian(T t) { return t; }
template <typename T> T byteswapToLittleEndian(T t) { return byteswap(t); }
#endif

} // namespace util
} // namespace lon