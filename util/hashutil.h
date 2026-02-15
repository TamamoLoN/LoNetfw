#pragma once

#include <algorithm>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <stdexcept>
#include <stdint.h>
#include <string.h>
#include <string>
#include <vector>

namespace lon
{
namespace util
{
class HashUtil
{
  public:
    static uint32_t murmur3_hash(const char *str, const uint32_t &seed = 1060627423);
    static uint64_t murmur3_hash64(const char *str, const uint32_t &seed = 1060627423,
                                   const uint32_t &seed2 = 1050126127);
    static uint32_t murmur3_hash(const void *str, const uint32_t &size,
                                 const uint32_t &seed = 1060627423);
    static uint64_t murmur3_hash64(const void *str, const uint32_t &size,
                                   const uint32_t &seed  = 1060627423,
                                   const uint32_t &seed2 = 1050126127);
    static uint32_t quick_hash(const char *str);
    static uint32_t quick_hash(const void *str, uint32_t size);

    static std::string base64decode(const std::string &src);
    static std::string base64encode(const std::string &data);
    static std::string base64encode(const void *data, size_t len);

    // Returns result in hex
    static std::string md5(const std::string &data);
    static std::string sha1(const std::string &data);
    // Returns result in blob
    static std::string md5sum(const std::string &data);
    static std::string md5sum(const void *data, size_t len);
#if OPENSSL_VERSION_NUMBER < 0x1010000fL
    static std::string sha0sum(const std::string &data);
    static std::string sha0sum(const void *data, size_t len);
#endif
    static std::string sha1sum(const std::string &data);
    static std::string sha1sum(const void *data, size_t len);
    static std::string hmac_md5(const std::string &text, const std::string &key);
    static std::string hmac_sha1(const std::string &text, const std::string &key);
    static std::string hmac_sha256(const std::string &text, const std::string &key);

    /// Output must be of size len * 2, and will *not* be null-terminated
    static void hexstring_from_data(const void *data, size_t len, char *output);
    static std::string hexstring_from_data(const void *data, size_t len);
    static std::string hexstring_from_data(const std::string &data);

    /// Output must be of size length / 2, and will *not* be null-terminated
    /// std::invalid_argument will be thrown if hexstring is not hex
    static void data_from_hexstring(const char *hexstring, size_t length, void *output);
    static std::string data_from_hexstring(const char *hexstring, size_t length);
    static std::string data_from_hexstring(const std::string &data);

    static void replace(std::string &str, char find, char replaceWith);
    static void replace(std::string &str, char find, const std::string &replaceWith);
    static void replace(std::string &str, const std::string &find, const std::string &replaceWith);

    static std::vector<std::string> split(const std::string &str, char delim, size_t max = ~0);
    static std::vector<std::string> split(const std::string &str, const char *delims,
                                          size_t max = ~0);

    static std::string random_string(size_t len);
};
} // namespace util
} // namespace lon