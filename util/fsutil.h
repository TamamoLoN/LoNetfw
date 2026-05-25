#pragma once

#include "util/endian.h"
#include "util/macro.h"

#ifdef _WIN32
#include <windows.h>

#include <direct.h>
#include <io.h>
#include <process.h>
#include <sys/stat.h>
#include <sys/types.h>
#define S_IFDIR _S_IFDIR
#define S_IRWXU _S_IREAD | _S_IWRITE | _S_IEXEC
#define S_IRWXG _S_IREAD | _S_IWRITE | _S_IEXEC
#define S_IROTH _S_IREAD
#define S_IXOTH _S_IEXEC
#define F_OK 0
#define X_OK 1
#define W_OK 2
#define R_OK 4
typedef struct _stat lon_stat_t;
#else
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct stat lon_stat_t;
#endif
#include <fstream>
#include <signal.h>
#include <string.h>
#include <string>
#include <vector>

namespace lon
{
namespace util
{
// 文件相关
class LON_API FSUtil
{
  public:
    static void getDirFiles(std::vector<std::string> &files, const std::string &dir_path,
                            const std::string &subfix);
    static bool mkdir(const std::string &dirname);
    static bool rm(const std::string &path);
    static bool mv(const std::string &from, const std::string &to);
    static bool realpath(const std::string &path, std::string &rpath);
    static bool symlink(const std::string &frm, const std::string &to);
    static bool unlink(const std::string &filename, bool exist = false);
    static std::string dirname(const std::string &filename);
    static std::string basename(const std::string &filename);
    static bool openRead(std::ifstream &ifs, const std::string &filename,
                         std::ios_base::openmode mode);
    static bool openWrite(std::ofstream &ofs, const std::string &filename,
                          std::ios_base::openmode mode);
    static bool isFileExist(const std::string &path);
    static size_t getFileSize(const std::string &path);
    static bool isProcRunning(const std::string &pidfile);

  private:
    static int __lstat(const char *file, lon_stat_t *st = nullptr);
    static int __mkdir(const char *dirname);
};

} // namespace util
} // namespace lon