#pragma once

#include "yaml-cpp/yaml.h"
#include <iostream>
#include <map>
#include <sstream>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>
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

template <typename T, typename S> T lexical_cast(const S &source)
{
    std::stringstream ss;
    T res;
    if (!(ss << source) || !(ss >> res) || !(ss >> std::ws).eof())
    {
        throw std::runtime_error("error cast: [" + ss.str() + "]");
    }

    return res;
}

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

uint32_t getThreadId();
uint32_t getFiberId();

} // namespace util

} // namespace lon