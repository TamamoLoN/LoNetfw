#pragma once

#include "util/endian.h"

#include <dirent.h>
#include <fstream>
#include <signal.h>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

namespace lon
{
namespace util
{
// 文件相关
class FSUtil
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
    static int __lstat(const char *file, struct stat *st = nullptr);
    static int __mkdir(const char *dirname);
};

} // namespace util
} // namespace lon