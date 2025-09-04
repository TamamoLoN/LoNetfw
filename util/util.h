#pragma once

#include "util/endian.h"
#include "util/lexicalcast.h"
#include "util/macro.h"
#include "util/noncopyable.h"
#include "util/singleton.h"
#include "yaml-cpp/yaml.h"
#include <assert.h>
#include <cxxabi.h>
#include <execinfo.h>
#include <functional>
#include <iostream>
#include <pthread.h>
#include <sstream>
#include <sys/syscall.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

namespace lon
{
namespace util
{
//字符串
char toLower(const char &ch);
std::string toLower(const std::string &str);
char toUpper(const char &ch);
std::string toUpper(const std::string &str);
// yaml风格的格式参数名验证规则
bool isValidParamName(const std::string &str);

void printYamlString(const YAML::Node &node, int layer = 0);
void convertYamlToVector(const std::string &prefix, const YAML::Node &node,
                         std::vector<std::pair<std::string, YAML::Node>> &vec);

template <class T> std::string getTypeStr()
{
    // abi::__cxa_demangle 获取的字符串需要手动释放内存
    char *type_str  = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
    std::string res = type_str;
    free(type_str);
    type_str = nullptr;
    return res;
}

enum Color
{
    UNKNOWN = -1,
    DEFAULT = 0,
    BLACK,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    PURPLE,
    CYAN,
    WHITE,
};
void getColorStr(std::string &str, Color color);

/**
 * 格式解析器
 * map<输出字符或给定格式: str,格式类型: uint8>
 * 格式类型0:str为非给定格式字符串；1:str为给定格式输出；2:str为时间日期格式
 */
std::vector<std::unordered_map<std::string, uint8_t>> formatParser(const std::string &str);

//日期时间
std::string getDateTime(const time_t &time, const std::string &format);
time_t getCurrentDateTime();
std::string getCurrentDateTime(const std::string &format);
uint64_t getCurrentMs();
uint64_t getCurrentUs();

uint32_t getThreadId();
std::string getThreadName();
uint32_t getFiberId();

//打印堆栈信息
void backtrace(std::vector<std::string> &bt, int32_t size, int32_t skip);
const std::string backtrace(int32_t size = 10, int32_t skip = 0, const std::string &prefix = "");

//内存分配
class Allocator
{
  public:
    static void *allocate(size_t size);
    static void deallocate(void *ptr);
};

class HookState
{
  public:
    static bool isEnable();
    static void enable();
    static void disable();
};

class ZigZag
{
    /**
     * 编码
     * 2:     00000000 00000010
     * -2:    11111111 11111110
     * -2<<1  11111111 11111100
     * -2>>15 11111111 11111111
     *  ^     00000000 00000011
     *
     * 解码
     * 2encode:       00000000 00000011
     * (2encode)>>1   00000000 00000001
     * -((2encode)&1) 11111111 11111111
     * ^              11111111 11111110
     * -2:            11111111 11111110
     */
  public:
    static uint16_t encode16(int16_t n);
    static int16_t decode16(uint16_t n);
    static uint32_t encode32(int32_t n);
    static int32_t decode32(uint32_t n);
    static uint64_t encode64(int64_t n);
    static int64_t decode64(uint64_t n);
};

class Varint
{
  public:
    static int8_t encode16(uint8_t *buf, uint16_t n);
    static int8_t encode32(uint8_t *buf, uint32_t n);
    static int8_t encode64(uint8_t *buf, uint64_t n);
};

} // namespace util

} // namespace lon
