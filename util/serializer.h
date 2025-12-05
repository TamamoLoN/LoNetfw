#pragma once

#include "util/endian.h"
#include <fstream>
#include <iomanip>
#include <math.h>
#include <memory>
#include <sstream>
#include <string.h>
#include <vector>
#ifdef _WIN32
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
struct iovec
{
    void *iov_base;
    size_t iov_len;
};
#else
#include <sys/uio.h>
#endif

namespace lon
{
namespace util
{

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

struct ByteArrayNode
{
    ByteArrayNode();
    ByteArrayNode(size_t s);
    ~ByteArrayNode();
    char *data;
    ByteArrayNode *next;
    size_t size;
};

class ByteArray
{
  public:
    using Ptr = std::shared_ptr<ByteArray>;
    ByteArray(size_t node_size = 4096);
    ~ByteArray();

    void writeFInt8(int8_t data);
    void writeFUInt8(uint8_t data);
    void writeFInt16(int16_t data);
    void writeFUInt16(uint16_t data);
    void writeFInt32(int32_t data);
    void writeFUInt32(uint32_t data);
    void writeFInt64(int64_t data);
    void writeFUInt64(uint64_t data);
    // 压缩
    void writeInt32(int32_t data);
    void writeUInt32(uint32_t data);
    void writeInt64(int64_t data);
    void writeUInt64(uint64_t data);

    void writeFloat(float data);
    void writeDouble(double data);

    // 长度: int16，以下同理
    void writeStringFInt16(const std::string &data);
    void writeStringFInt32(const std::string &data);
    void writeStringFInt64(const std::string &data);
    void writeStringVInt(const std::string &data);
    // 长度: 可变
    void writeString(const std::string &data);

    int8_t readFInt8();
    uint8_t readFUInt8();
    int16_t readFInt16();
    uint16_t readFUInt16();
    int32_t readFInt32();
    uint32_t readFUInt32();
    int64_t readFInt64();
    uint64_t readFUInt64();

    int32_t readInt32();
    uint32_t readUInt32();
    int64_t readInt64();
    uint64_t readUInt64();

    float readFloat();
    double readDouble();

    std::string readStringFInt16();
    std::string readStringFInt32();
    std::string readStringFInt64();
    std::string readStringVInt();

    void clear();
    void write(const void *buf, size_t size);
    void read(void *buf, size_t size);
    void read(void *buf, size_t size, size_t position) const;

    size_t getPosition();
    void setPosition(size_t pos);
    size_t getNodeSize();
    // 获取当前还有多少数据可以读
    size_t getReadSize();
    // 返回当前节点的数量
    size_t count() const;
    size_t size() const;

    // 获取bytearray的字节序，而不是系统本身的字节序
    bool isLittleEndian();
    // 设置bytearray的字节序，而不是系统本身的字节序
    void setLittleEndian(bool is_little);

    bool writeToFile(const std::string &path);
    bool readFromFile(const std::string &path);

    std::string toString() const;
    std::string toStringHex() const;

    // 获取可读缓存
    size_t getReadBuffers(std::vector<iovec> &bufs, size_t len = -0ull) const;
    size_t getReadBuffers(std::vector<iovec> &bufs, size_t len, size_t position) const;
    // 获取可写缓存
    size_t getWriteBuffers(std::vector<iovec> &bufs, size_t len);

  private:
    // 扩容
    void addTotalSize(size_t size);
    // 获取剩余容量大小
    size_t getRemainSize();

  private:
    ByteArrayNode *m_root;
    ByteArrayNode *m_cur;
    // 每一个节点的大小
    size_t m_node_size;
    // 当前总的容量大小
    size_t m_total_size;
    // 当前数据大小
    size_t m_data_size;
    // 当前操作位置
    size_t m_position;
    // bytearray使用的字节序
    uint8_t m_ba_endian;
};

class Serializer
{
  public:
    Serializer();
    ~Serializer();
};
} // namespace util
} // namespace lon
