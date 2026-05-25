#include "util/fsutil.h"

namespace lon
{
namespace util
{
void FSUtil::getDirFiles(std::vector<std::string> &files, const std::string &dir_path,
                         const std::string &subfix)
{
#ifdef _WIN32
    std::string find_path = dir_path + "/*";
    WIN32_FIND_DATAA find_data;
    HANDLE hFind = FindFirstFileA(find_path.c_str(), &find_data);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        return;
    }
    do
    {
        if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (strcmp(find_data.cFileName, ".") == 0 || strcmp(find_data.cFileName, "..") == 0)
            {
                continue;
            }
            getDirFiles(files, dir_path + "/" + find_data.cFileName, subfix);
        }
        else
        {
            std::string filename(find_data.cFileName);
            if (subfix.empty())
            {
                files.push_back(dir_path + "/" + filename);
            }
            else
            {
                if (filename.size() < subfix.size())
                {
                    continue;
                }
                if (filename.substr(filename.length() - subfix.size()) == subfix)
                {
                    files.push_back(dir_path + "/" + filename);
                }
            }
        }
    } while (FindNextFileA(hFind, &find_data) != 0);
    FindClose(hFind);
#else
    if (access(dir_path.c_str(), 0) != 0)
    {
        return;
    }
    DIR *dir = opendir(dir_path.c_str());
    if (dir == nullptr)
    {
        return;
    }
    struct dirent *dp = nullptr;
    while ((dp = readdir(dir)) != nullptr)
    {
        if (dp->d_type == DT_DIR)
        {
            if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
            {
                continue;
            }
            getDirFiles(files, dir_path + "/" + dp->d_name, subfix);
        }
        else if (dp->d_type == DT_REG)
        {
            std::string filename(dp->d_name);
            if (subfix.empty())
            {
                files.push_back(dir_path + "/" + filename);
            }
            else
            {
                if (filename.size() < subfix.size())
                {
                    continue;
                }
                if (filename.substr(filename.length() - subfix.size()) == subfix)
                {
                    files.push_back(dir_path + "/" + filename);
                }
            }
        }
    }
    closedir(dir);
#endif
}

bool FSUtil::mkdir(const std::string &dirname)
{
    if (__lstat(dirname.c_str()) == 0)
    {
        return true;
    }
    char *path = strdup(dirname.c_str());
    char *ptr  = strchr(path + 1, '/');
    do
    {
        for (; ptr; *ptr = '/', ptr = strchr(ptr + 1, '/'))
        {
            *ptr = '\0';
            if (__mkdir(path) != 0)
            {
                break;
            }
        }
        if (ptr != nullptr)
        {
            break;
        }
        else if (__mkdir(path) != 0)
        {
            break;
        }
        free(path);
        return true;
    } while (0);
    free(path);
    return false;
}

bool FSUtil::isProcRunning(const std::string &pidfile)
{
    if (__lstat(pidfile.c_str()) != 0)
    {
        return false;
    }
    std::ifstream ifs(pidfile);
    std::string line;
    if (!ifs || !std::getline(ifs, line))
    {
        return false;
    }
    if (line.empty())
    {
        return false;
    }
    int32_t pid = atoi(line.c_str());
    if (pid <= 1)
    {
        return false;
    }
#ifdef _WIN32
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (hProcess == NULL)
    {
        return false;
    }
    DWORD exitCode;
    BOOL result = GetExitCodeProcess(hProcess, &exitCode);
    CloseHandle(hProcess);
    return result && (exitCode == STILL_ACTIVE);
#else
    if (kill(pid, 0) != 0)
    {
        return false;
    }
    return true;
#endif
}

bool FSUtil::unlink(const std::string &filename, bool exist)
{
    if (!exist && __lstat(filename.c_str()))
    {
        return true;
    }
#ifdef _WIN32
    return _unlink(filename.c_str()) == 0;
#else
    return ::unlink(filename.c_str()) == 0;
#endif
}

bool FSUtil::rm(const std::string &path)
{
    lon_stat_t st;
#ifdef _WIN32
    if (_stat(path.c_str(), &st))
#else
    if (lstat(path.c_str(), &st))
#endif
    {
        return true;
    }
    if (!(st.st_mode & S_IFDIR))
    {
        return unlink(path);
    }

#ifdef _WIN32
    std::string find_path = path + "/*";
    WIN32_FIND_DATAA find_data;
    HANDLE hFind = FindFirstFileA(find_path.c_str(), &find_data);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    bool ret = true;
    do
    {
        if (strcmp(find_data.cFileName, ".") == 0 || strcmp(find_data.cFileName, "..") == 0)
        {
            continue;
        }
        std::string dirname = path + "/" + find_data.cFileName;
        ret                 = rm(dirname);
    } while (FindNextFileA(hFind, &find_data) != 0);
    FindClose(hFind);
#else
    DIR *dir = opendir(path.c_str());
    if (!dir)
    {
        return false;
    }

    bool ret          = true;
    struct dirent *dp = nullptr;
    while ((dp = readdir(dir)))
    {
        if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
        {
            continue;
        }
        std::string dirname = path + "/" + dp->d_name;
        ret                 = rm(dirname);
    }
    closedir(dir);
#endif
#ifdef _WIN32
    if (_rmdir(path.c_str()))
    {
        ret = false;
    }
#else
    if (::rmdir(path.c_str()))
    {
        ret = false;
    }
#endif
    return ret;
}

bool FSUtil::mv(const std::string &from, const std::string &to)
{
    if (!rm(to))
    {
        return false;
    }
    return ::rename(from.c_str(), to.c_str()) == 0;
}

bool FSUtil::realpath(const std::string &path, std::string &rpath)
{
    if (__lstat(path.c_str()))
    {
        return false;
    }
#ifdef _WIN32
    char *ptr = _fullpath(nullptr, path.c_str(), 0);
#else
    char *ptr = ::realpath(path.c_str(), nullptr);
#endif
    if (nullptr == ptr)
    {
        return false;
    }
    std::string(ptr).swap(rpath);
    free(ptr);
    return true;
}

bool FSUtil::symlink(const std::string &from, const std::string &to)
{
    if (!rm(to))
    {
        return false;
    }
#ifdef _WIN32
    return CreateSymbolicLinkA(to.c_str(), from.c_str(), 0) != 0;
#else
    return ::symlink(from.c_str(), to.c_str()) == 0;
#endif
}

std::string FSUtil::dirname(const std::string &filename)
{
    if (filename.empty())
    {
        return ".";
    }
    auto pos = filename.rfind('/');
    if (pos == 0)
    {
        return "/";
    }
    else if (pos == std::string::npos)
    {
        return ".";
    }
    else
    {
        return filename.substr(0, pos);
    }
}

std::string FSUtil::basename(const std::string &filename)
{
    if (filename.empty())
    {
        return filename;
    }
    auto pos = filename.rfind('/');
    if (pos == std::string::npos)
    {
        return filename;
    }
    else
    {
        return filename.substr(pos + 1);
    }
}

bool FSUtil::openRead(std::ifstream &ifs, const std::string &filename, std::ios_base::openmode mode)
{
    ifs.open(filename.c_str(), mode);
    return ifs.is_open();
}

bool FSUtil::openWrite(std::ofstream &ofs, const std::string &filename,
                       std::ios_base::openmode mode)
{
    ofs.open(filename.c_str(), mode);
    if (!ofs.is_open())
    {
        std::string dir = dirname(filename);
        mkdir(dir);
        ofs.open(filename.c_str(), mode);
    }
    return ofs.is_open();
}

bool FSUtil::isFileExist(const std::string &path)
{
    lon_stat_t buffer;
#ifdef _WIN32
    return (_stat(path.c_str(), &buffer) == 0);
#else
    return (stat(path.c_str(), &buffer) == 0);
#endif
}

size_t FSUtil::getFileSize(const std::string &path)
{
    lon_stat_t st;
#ifdef _WIN32
    if (_stat(path.c_str(), &st) == 0)
#else
    if (stat(path.c_str(), &st) == 0)
#endif
    {
        return st.st_size;
    }
    else
    {
        throw std::runtime_error("getFileSize: stat file failed, path: " + path);
    }
}

int FSUtil::__lstat(const char *file, lon_stat_t *st)
{
    lon_stat_t lst;
#ifdef _WIN32
    int ret = ::_stat(file, &lst);
#else
    int ret = ::lstat(file, &lst);
#endif
    if (st)
    {
        *st = lst;
    }
    return ret;
}

int FSUtil::__mkdir(const char *dirname)
{
#ifdef _WIN32
    if (::_access(dirname, F_OK) == 0)
    {
        return 0;
    }
    return ::_mkdir(dirname);
#else
    if (::access(dirname, F_OK) == 0)
    {
        return 0;
    }
    return ::mkdir(dirname, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
}

} // namespace util
} // namespace lon