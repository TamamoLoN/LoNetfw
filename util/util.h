#pragma once

#include "util/endian.h"
#include "util/lexicalcast.h"
#include "util/macro.h"
#include "util/noncopyable.h"
#include "util/serializer.h"
#include "util/singleton.h"
#include "util/stream.h"

#include "util/buffer.h"
#include "yaml-cpp/yaml.h"
#include <assert.h>
#include <functional>
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <time.h>

#ifdef _WIN32
#define _TIMESPEC_DEFINED
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>

#include <Windows.h>
#include <profileapi.h>
#include <pthread.h>
#include <shlwapi.h>
#include <sysinfoapi.h>
#undef ERROR
#include <DbgHelp.h>
#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "Shlwapi.lib")
int vasprintf(char **buf, const char *fmt, va_list ap);
#else
#include <cxxabi.h>
#include <execinfo.h>
#include <fnmatch.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <unistd.h>
#endif

namespace lon
{
namespace util
{
// 字符串
char toLower(const char &ch);
std::string toLower(const std::string &str);
char toUpper(const char &ch);
std::string toUpper(const std::string &str);
std::vector<std::string> split(const std::string &s, const std::string &delimiter);
std::string trim(const std::string &str);
bool globMatch(const std::string &pattern, const std::string &text);

// 文件相关
bool isFileExist(const std::string &path);
size_t getFileSize(const std::string &path);

// yaml风格的格式参数名验证规则
bool isValidParamName(const std::string &str);
void printYamlString(const YAML::Node &node, int layer = 0);
void convertYamlToVector(const std::string &prefix, const YAML::Node &node,
                         std::vector<std::pair<std::string, YAML::Node>> &vec);

template <class T> std::string getTypeStr()
{
#ifdef _WIN32
    const char *name = typeid(T).name(); // 已经是 MSVC 装饰名
    char undec[1024];
    if (UnDecorateSymbolName(name, undec, sizeof(undec), UNDNAME_COMPLETE))
    {
        return std::string(undec);
    }
    // 解码失败就返回原名
    return std::string(name);
#else
    // abi::__cxa_demangle 获取的字符串需要手动释放内存
    char *type_str  = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
    std::string res = type_str;
    free(type_str);
    type_str = nullptr;
    return res;
#endif
}

struct InsensitiveStringCompare
{
    bool operator()(const std::string &lhs, const std::string &rhs) const;
};

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

// 日期时间
std::string getDateTime(const time_t &time, const std::string &format);
time_t getCurrentDateTime();
std::string getCurrentDateTime(const std::string &format);
uint64_t getCurrentMs();
uint64_t getCurrentUs();
uint64_t getDurationUs(std::function<void()> func);

uint32_t getThreadId();
std::string getThreadName();
uint32_t getFiberId();

// 打印堆栈信息
void backtrace(std::vector<std::string> &bt, int32_t size, int32_t skip);
const std::string backtrace(int32_t size = 10, int32_t skip = 0, const std::string &prefix = "");

// 内存分配
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
} // namespace util

} // namespace lon
