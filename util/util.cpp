#include "util/util.h"

#ifdef _WIN32
int vasprintf(char **buf, const char *fmt, va_list ap)
{
    va_list ap_copy;
    va_copy(ap_copy, ap);

    int len = _vscprintf(fmt, ap_copy);
    va_end(ap_copy);

    if (len < 0)
    {
        return -1;
    }

    *buf = (char *)malloc(len + 1);
    if (!*buf)
    {
        return -1;
    }

    int written = vsnprintf(*buf, len + 1, fmt, ap);
    if (written < 0)
    {
        free(*buf);
        return -1;
    }

    return written;
}
#endif

namespace lon
{
namespace util
{
char toLower(const char &ch)
{
    if (ch >= 'A' && ch <= 'Z')
    {
        return (char)(ch + 32);
    }
    else
    {
        return ch;
    }
}

std::string toLower(const std::string &str)
{
    std::string res = str;
    for (auto &ch : res)
    {
        ch = toLower(ch);
    }
    return res;
}

char toUpper(const char &ch)
{
    if (ch >= 'a' && ch <= 'z')
    {
        return (char)(ch - 32);
    }
    else
    {
        return ch;
    }
}

std::string toUpper(const std::string &str)
{
    std::string res = str;
    for (auto &ch : res)
    {
        ch = toUpper(ch);
    }
    return res;
}

std::vector<std::string> split(const std::string &s, const std::string &delimiter)
{
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end   = 0;

    while ((end = s.find(delimiter, start)) != std::string::npos)
    {
        tokens.push_back(s.substr(start, end - start));
        start = end + delimiter.length();
    }

    tokens.push_back(s.substr(start));
    return tokens;
}

std::string trim(const std::string &str)
{
    size_t start = 0;
    size_t end   = str.size();
    while (start < end && std::isspace(static_cast<unsigned char>(str[start])))
    {
        ++start;
    }
    while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1])))
    {
        --end;
    }
    return str.substr(start, end - start);
}

bool isFileExist(const std::string &path)
{
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

size_t getFileSize(const std::string &path)
{
    struct stat st;
    if (stat(path.c_str(), &st) == 0)
    {
        return st.st_size;
    }
    else
    {
        throw std::runtime_error("getFileSize: stat file failed, path: " + path);
    }
}

bool isValidParamName(const std::string &str)
{
    if (str.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.") !=
        std::string::npos)
    {
        return false;
    }
    return true;
}

void printYamlString(const YAML::Node &node, int layer)
{
    if (node.IsNull())
    {
        std::stringstream ss;
        ss << std::string(layer * 2, ' ') << "Null -" << node.Type() << "-" << layer << std::endl;
        std::cout << ss.str();
    }
    else if (node.IsScalar())
    {
        std::stringstream ss;
        ss << std::string(layer * 2, ' ') << node.Scalar() << "-" << node.Type() << "-" << layer
           << std::endl;
        std::cout << ss.str();
    }
    else if (node.IsMap())
    {
        for (auto it = node.begin(); it != node.end(); ++it)
        {
            std::stringstream ss;
            ss << std::string(layer * 2, ' ') << it->first << "-" << it->second.Type() << "-"
               << layer << std::endl;
            std::cout << ss.str();
            printYamlString(it->second, layer + 1);
        }
    }
    else if (node.IsSequence())
    {
        for (int cnt = 0; cnt < node.size(); ++cnt)
        {
            std::stringstream ss;
            ss << std::string(layer * 2, ' ') << cnt << "-" << node[cnt].Type() << "-" << layer
               << std::endl;
            std::cout << ss.str();
            printYamlString(node[cnt], layer + 1);
        }
    }
}

void convertYamlToVector(const std::string &prefix, const YAML::Node &node,
                         std::vector<std::pair<std::string, YAML::Node>> &vec)
{
    if (!isValidParamName(toLower(prefix)))
    {
        throw std::runtime_error("data name is invalid: " + prefix);
    }
    vec.push_back(std::make_pair(toLower(prefix), node));
    if (node.IsMap())
    {
        for (auto it = node.begin(); it != node.end(); ++it)
        {
            convertYamlToVector(prefix.empty()
                                    ? it->first.Scalar()
                                    : std::string(toLower(prefix) + "." + it->first.Scalar()),
                                it->second, vec);
        }
    }
}

bool InsensitiveStringCompare::operator()(const std::string &lhs, const std::string &rhs) const
{
    return toLower(lhs) < toLower(rhs);
}

void getColorStr(std::string &str, Color color)
{
    auto color_num = (int)color;
    if (color_num < 1)
    {
        return;
    }
    std::string front = "\033[" + std::to_string(29 + color_num) + "m";
    std::string back  = "\033[0m";
    str               = front + str + back;
}

std::vector<std::unordered_map<std::string, uint8_t>> formatParser(const std::string &str)
{
    std::vector<std::unordered_map<std::string, uint8_t>> res;
    for (int i = 0; i < str.size(); ++i)
    {
        if (str[i] != '%')
        {
            std::string temp = "";
            while (str[i] != '%' && i < str.size())
            {
                temp += str[i];
                ++i;
            }
            std::unordered_map<std::string, uint8_t> m;
            m[temp] = 0;
            res.push_back(m);
            i--;
        }
        else
        {
            if ((i + 1) < str.size() && str[i + 1] != '%')
            {
                if ((i + 2) < str.size() && str[i + 1] == 'd' && str[i + 2] == '{')
                {
                    i += 3;
                    std::string temp = "";
                    while (str[i] != '}' && i < str.size())
                    {
                        temp += str[i];
                        ++i;
                    }
                    if (i >= str.size())
                    {
                        throw std::runtime_error(temp + ": format error");
                        break;
                    }
                    std::unordered_map<std::string, uint8_t> m;
                    m[temp] = 2;
                    res.push_back(m);
                }
                else
                {
                    std::unordered_map<std::string, uint8_t> m;
                    std::string temp = "";
                    temp += str[i + 1];
                    m[temp] = 1;
                    res.push_back(m);
                    ++i;
                }
            }
            else
            {
                std::unordered_map<std::string, uint8_t> m;
                m["%"] = 0;
                res.push_back(m);
            }
        }
    }
    return res;
}

std::string getDateTime(const time_t &time, const std::string &format)
{
    struct tm tm;
#ifdef _WIN32
    localtime_s(&tm, &time);
#else
    localtime_r(&time, &tm);
#endif
    char buf[64] = {0};
    strftime(buf, sizeof(buf), format.c_str(), &tm);
    return std::string(buf);
}

time_t getCurrentDateTime() { return time(nullptr); }

std::string getCurrentDateTime(const std::string &format)
{
    return getDateTime(getCurrentDateTime(), format);
}

uint64_t getCurrentMs()
{
#ifdef _WIN32
    static LARGE_INTEGER freq;
    static BOOL inited = QueryPerformanceFrequency(&freq);

    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return now.QuadPart * 1000 / freq.QuadPart;
#else
#ifdef USE_HIGH_PRECISION_TIME
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000 / 1000;
#else
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
#endif
}

uint64_t getCurrentUs()
{
#ifdef _WIN32
    static LARGE_INTEGER freq;
    static BOOL inited = QueryPerformanceFrequency(&freq);

    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return now.QuadPart * 1000000 / freq.QuadPart;
#else
#ifdef USE_HIGH_PRECISION_TIME
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000 * 1000ul + ts.tv_nsec / 1000;
#else
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec * 1000 * 1000ul + tv.tv_usec;
#endif
#endif
}

uint64_t getDurationUs(std::function<void()> func)
{
    uint64_t start = getCurrentUs();
    func();
    return getCurrentUs() - start;
}

uint32_t getThreadId()
{
#ifdef _WIN32
    return GetCurrentThreadId();
#else
    return syscall(SYS_gettid);
#endif
}

std::string getThreadName()
{
#ifdef _WIN32
    auto WideToUtf8 = [](const std::wstring &src) -> std::string
    {
        if (src.empty())
        {
            return "";
        }
        int size_needed =
            WideCharToMultiByte(CP_UTF8, 0, src.c_str(), -1, nullptr, 0, nullptr, nullptr);
        std::string ret(size_needed - 1, 0);
        WideCharToMultiByte(CP_UTF8, 0, src.c_str(), -1, &ret[0], size_needed, nullptr, nullptr);
        return ret;
    };
    // 尝试获取线程名
    PWSTR wthread = nullptr;
    HRESULT hr    = GetThreadDescription(GetCurrentThread(), &wthread);
    if (SUCCEEDED(hr) && wthread && wcslen(wthread) > 0)
    {
        std::string name = WideToUtf8(wthread);
        LocalFree(wthread);
        if (!name.empty())
        {
            return name;
        }
    }
    if (wthread)
    {
        LocalFree(wthread);
    }
    // 获取进程名
    wchar_t wpath[MAX_PATH] = {0};
    DWORD len               = GetModuleFileNameW(NULL, wpath, MAX_PATH);
    if (len == 0)
    {
        return "";
    }
    std::wstring wfile(wpath);
    size_t pos         = wfile.find_last_of(L"\\/");
    std::wstring wname = (pos == std::wstring::npos) ? wfile : wfile.substr(pos + 1);

    return WideToUtf8(wname);
#else
    char buf[16] = {0};
    pthread_getname_np(pthread_self(), buf, sizeof(buf));
    return std::string(buf);
#endif
}

// TODO - 实现获取协程id
uint32_t getFiberId() { return 0; }

void backtrace(std::vector<std::string> &bt, int32_t size, int32_t skip)
{
#ifdef _WIN32
    static HANDLE process           = GetCurrentProcess();
    static bool dbghelp_initialized = false;
    static CRITICAL_SECTION cs;

    if (!dbghelp_initialized)
    {
        InitializeCriticalSection(&cs);
        EnterCriticalSection(&cs);

        SymInitialize(process, NULL, TRUE);
        SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
        dbghelp_initialized = true;

        LeaveCriticalSection(&cs);
    }

    void *stack[64];
    USHORT frames = CaptureStackBackTrace(skip, size, stack, NULL);

    EnterCriticalSection(&cs);

    for (USHORT i = 0; i < frames; i++)
    {
        DWORD64 addr = (DWORD64)stack[i];

        // --- 解析符号名 ---
        char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME];
        SYMBOL_INFO *symbol  = (SYMBOL_INFO *)buffer;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen   = MAX_SYM_NAME;

        DWORD64 displacement = 0;
        BOOL has_symbol      = SymFromAddr(process, addr, &displacement, symbol);

        // --- 解析模块名 ---
        IMAGEHLP_MODULE64 module_info;
        memset(&module_info, 0, sizeof(module_info));
        module_info.SizeOfStruct = sizeof(module_info);
        SymGetModuleInfo64(process, addr, &module_info);

        char line[512];
        if (has_symbol)
        {
            snprintf(line, sizeof(line), "%s!%s + 0x%llx",
                     module_info.ImageName ? module_info.ImageName : "unknown", symbol->Name,
                     displacement);
        }
        else
        {
            snprintf(line, sizeof(line), "0x%llx", addr);
        }

        bt.push_back(line);
    }

    LeaveCriticalSection(&cs);
#else
    // 协程会使用（栈设置的很小），尽量不在栈上分配内存，防止栈溢出
    void **array   = (void **)malloc(sizeof(void *) * size);
    int32_t s      = ::backtrace(array, size);
    char **strings = backtrace_symbols(array, s);
    if (strings == nullptr)
    {
        free(array);
        throw std::runtime_error("backtrace error");
    }
    for (int32_t i = skip; i < s; ++i)
    {
        bt.push_back(strings[i]);
    }
    free(array);
    free(strings);
#endif
}

const std::string backtrace(int32_t size, int32_t skip, const std::string &prefix)
{
    std::vector<std::string> bt;
    backtrace(bt, size, skip);
    std::stringstream ss;
    for (auto &s : bt)
    {
        ss << prefix << s << std::endl;
    }
    return ss.str();
}

void *Allocator::allocate(size_t size) { return malloc(size); }

void Allocator::deallocate(void *ptr) { free(ptr); }

static thread_local bool t_is_hook_enabled = false;

bool HookState::isEnable() { return t_is_hook_enabled; }

void HookState::enable() { t_is_hook_enabled = true; }

void HookState::disable() { t_is_hook_enabled = false; }

} // namespace util
} // namespace lon