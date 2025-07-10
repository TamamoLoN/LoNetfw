#pragma once

#include <iostream>
#include <time.h>
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
} // namespace util

} // namespace lon