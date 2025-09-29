#include "util/serializer.h"

namespace lon
{
namespace util
{
uint16_t ZigZag::encode16(int16_t n) { return (n << 1) ^ (n >> (sizeof(n) * 8 - 1)); }

int16_t ZigZag::decode16(uint16_t n) { return (n >> 1) ^ -(n & 1); }

uint32_t ZigZag::encode32(int32_t n) { return (n << 1) ^ (n >> (sizeof(n) * 8 - 1)); }

int32_t ZigZag::decode32(uint32_t n) { return (n >> 1) ^ -(n & 1); }

uint64_t ZigZag::encode64(int64_t n) { return (n << 1) ^ (n >> (sizeof(n) * 8 - 1)); }

int64_t ZigZag::decode64(uint64_t n) { return (n >> 1) ^ -(n & 1); }

int8_t Varint::encode16(uint8_t *buf, uint16_t n)
{
    uint8_t cnt = 0;
    if (!buf)
    {
        return -1;
    }
    while (n > 0b01111111)
    {
        //从字节流末尾取出 7 bit 并在最高位增加 1 构成一个字节
        buf[cnt] = (n & 0b01111111) | 0b10000000;
        n >>= 7;
        ++cnt;
    }
    // 如果是最后一个字节增加 0
    buf[cnt] = n;
    return cnt + 1;
}

int8_t Varint::encode32(uint8_t *buf, uint32_t n)
{
    uint8_t cnt = 0;
    if (!buf)
    {
        return -1;
    }
    while (n > 0b01111111)
    {
        buf[cnt] = (n & 0b01111111) | 0b10000000;
        n >>= 7;
        ++cnt;
    }
    buf[cnt] = n;
    return cnt + 1;
}

int8_t Varint::encode64(uint8_t *buf, uint64_t n)
{
    uint8_t cnt = 0;
    if (!buf)
    {
        return -1;
    }
    while (n > 0b01111111)
    {
        buf[cnt] = (n & 0b01111111) | 0b10000000;
        n >>= 7;
        ++cnt;
    }
    buf[cnt] = n;
    return cnt + 1;
}

ByteArrayNode::ByteArrayNode() : data(nullptr), next(nullptr), size(0) {}

ByteArrayNode::ByteArrayNode(size_t s) : data(new char[s]), next(nullptr), size(s) {}

ByteArrayNode::~ByteArrayNode()
{
    if (data)
    {
        delete[] data;
    }
}

ByteArray::ByteArray(size_t node_size)
    : m_node_size(node_size), m_position(0), m_total_size(node_size), m_data_size(0),
      m_root(new ByteArrayNode(node_size)), m_cur(m_root), m_ba_endian(LON_BIG_ENDIAN)
{
    // TODO
}

ByteArray::~ByteArray()
{
    auto tmp = m_root;
    while (tmp != nullptr)
    {
        m_cur = tmp;
        tmp   = tmp->next;
        delete m_cur;
        m_cur = nullptr;
    }
}

void ByteArray::writeFInt8(int8_t data) { write(&data, sizeof(data)); }

void ByteArray::writeFUInt8(uint8_t data) { write(&data, sizeof(data)); }

void ByteArray::writeFInt16(int16_t data)
{
    data = (m_ba_endian == LON_ENDIAN) ? data : util::byteswap(data);
    write(&data, sizeof(data));
}

void ByteArray::writeFUInt16(uint16_t data)
{
    data = (m_ba_endian == LON_ENDIAN) ? data : util::byteswap(data);
    write(&data, sizeof(data));
}

void ByteArray::writeFInt32(int32_t data)
{
    data = (m_ba_endian == LON_ENDIAN) ? data : util::byteswap(data);
    write(&data, sizeof(data));
}

void ByteArray::writeFUInt32(uint32_t data)
{
    data = (m_ba_endian == LON_ENDIAN) ? data : util::byteswap(data);
    write(&data, sizeof(data));
}

void ByteArray::writeFInt64(int64_t data)
{
    data = (m_ba_endian == LON_ENDIAN) ? data : util::byteswap(data);
    write(&data, sizeof(data));
}

void ByteArray::writeFUInt64(uint64_t data)
{
    data = (m_ba_endian == LON_ENDIAN) ? data : util::byteswap(data);
    write(&data, sizeof(data));
}

void ByteArray::writeInt32(int32_t data) { writeUInt32(util::ZigZag::encode32(data)); }

void ByteArray::writeUInt32(uint32_t data)
{
    uint8_t buf[5];
    uint8_t len = util::Varint::encode32(buf, data);
    write(buf, len);
}

void ByteArray::writeInt64(int64_t data) { writeUInt64(util::ZigZag::encode64(data)); }

void ByteArray::writeUInt64(uint64_t data)
{
    uint8_t buf[10];
    uint8_t len = util::Varint::encode64(buf, data);
    write(buf, len);
}

void ByteArray::writeFloat(float data)
{
    uint32_t tmp = 0;
    memcpy(&tmp, &data, sizeof(float));
    writeFUInt32(data);
}

void ByteArray::writeDouble(double data)
{
    uint64_t tmp = 0;
    memcpy(&tmp, &data, sizeof(float));
    writeFUInt64(data);
}

void ByteArray::writeStringFInt16(const std::string &data)
{
    writeFUInt16(data.size());
    write(data.c_str(), data.size());
}

void ByteArray::writeStringFInt32(const std::string &data)
{
    writeFUInt32(data.size());
    write(data.c_str(), data.size());
}

void ByteArray::writeStringFInt64(const std::string &data)
{
    writeFUInt64(data.size());
    write(data.c_str(), data.size());
}

void ByteArray::writeStringVInt(const std::string &data)
{
    writeUInt64(data.size());
    write(data.c_str(), data.size());
}

void ByteArray::writeString(const std::string &data) { write(data.c_str(), data.size()); }

int8_t ByteArray::readFInt8()
{
    int8_t data = 0;
    read(&data, sizeof(data));
    return data;
}

uint8_t ByteArray::readFUInt8()
{
    uint8_t data = 0;
    read(&data, sizeof(data));
    return data;
}

int16_t ByteArray::readFInt16()
{
    int16_t data = 0;
    read(&data, sizeof(data));
    return (m_ba_endian == LON_ENDIAN) ? data : util::byteswap(data);
}

uint16_t ByteArray::readFUInt16()
{
    uint16_t data = 0;
    read(&data, sizeof(data));
    return (m_ba_endian == LON_ENDIAN) ? data : util::byteswap(data);
}

int32_t ByteArray::readFInt32()
{
    int32_t data = 0;
    read(&data, sizeof(data));
    return (m_ba_endian == LON_ENDIAN) ? data : util::byteswap(data);
}

uint32_t ByteArray::readFUInt32()
{
    uint32_t data = 0;
    read(&data, sizeof(data));
    return (m_ba_endian == LON_ENDIAN) ? data : util::byteswap(data);
}

int64_t ByteArray::readFInt64()
{
    int64_t data = 0;
    read(&data, sizeof(data));
    return (m_ba_endian == LON_ENDIAN) ? data : util::byteswap(data);
}

uint64_t ByteArray::readFUInt64()
{
    uint64_t data = 0;
    read(&data, sizeof(data));
    return (m_ba_endian == LON_ENDIAN) ? data : util::byteswap(data);
}

int32_t ByteArray::readInt32() { return util::ZigZag::decode32(readUInt32()); }

uint32_t ByteArray::readUInt32()
{
    uint32_t data = 0;
    for (uint8_t cnt = 0; cnt < 32; cnt += 7)
    {
        uint8_t tmp = readFUInt8();
        if (tmp < 0b10000000)
        {
            data |= ((uint32_t)tmp << cnt);
            break;
        }
        else
        {
            data |= ((uint32_t)(tmp & 0b01111111) << cnt);
        }
    }
    return data;
}

int64_t ByteArray::readInt64() { return util::ZigZag::decode64(readUInt64()); }

uint64_t ByteArray::readUInt64()
{
    uint64_t data = 0;
    for (uint8_t cnt = 0; cnt < 64; cnt += 7)
    {
        uint8_t tmp = readFUInt8();
        if (tmp < 0b10000000)
        {
            data |= ((uint64_t)tmp << cnt);
            break;
        }
        else
        {
            data |= ((uint64_t)(tmp & 0b01111111) << cnt);
        }
    }
    return data;
}

float ByteArray::readFloat()
{
    uint32_t data = readFUInt32();
    float res     = 0.0f;
    memcpy(&res, &data, sizeof(data));
    return res;
}

double ByteArray::readDouble()
{
    uint64_t data = readFUInt64();
    double res     = 0.0;
    memcpy(&res, &data, sizeof(data));
    return res;
}

std::string ByteArray::readStringFInt16()
{
    uint16_t len = readFUInt16();
    std::string res;
    res.resize(len);
    read(&res[0], len);
    return res;
}

std::string ByteArray::readStringFInt32()
{
    uint32_t len = readFUInt32();
    std::string res;
    res.resize(len);
    read(&res[0], len);
    return res;
}

std::string ByteArray::readStringFInt64()
{
    uint64_t len = readFUInt64();
    std::string res;
    res.resize(len);
    read(&res[0], len);
    return res;
}

std::string ByteArray::readStringVInt()
{
    uint64_t len = readUInt64();
    std::string res;
    res.resize(len);
    read(&res[0], len);
    return res;
}

void ByteArray::clear()
{
    m_position   = 0;
    m_data_size  = 0;
    m_total_size = m_node_size;
    auto tmp     = m_root->next;
    while (tmp != nullptr)
    {
        m_cur = tmp;
        tmp   = tmp->next;
        delete m_cur;
        m_cur = nullptr;
    }
    m_cur        = m_root;
    m_root->next = nullptr;
}

void ByteArray::write(const void *buf, size_t size)
{
    if (size == 0)
    {
        return;
    }
    addTotalSize(size);
    size_t node_pos    = m_position % m_node_size; // 在当前node中的位置
    size_t node_remain = m_cur->size - node_pos;   // 当前node剩余大小
    size_t buf_pos     = 0;                        // 当前buf写到的位置

    while (size > 0)
    {
        if (node_remain >= size)
        {
            memcpy(m_cur->data + node_pos, (char *)buf + buf_pos, size);
            if (m_cur->size == (node_pos + size))
            {
                m_cur = m_cur->next;
            }
            m_position += size;
            buf_pos += size;
            size = 0;
        }
        else
        {
            memcpy(m_cur->data + node_pos, (char *)buf + buf_pos, node_remain);
            m_position += node_remain;
            buf_pos += node_remain;
            size -= node_remain;
            m_cur       = m_cur->next;
            node_remain = m_cur->size;
            node_pos    = 0;
        }
    }
    if (m_position > m_data_size)
    {
        m_data_size = m_position;
    }
}

void ByteArray::read(void *buf, size_t size)
{
    if (size == 0)
    {
        return;
    }
    if (size > getReadSize())
    {
        throw std::runtime_error(
            "ByteArray::read: read out of range, m_data_size: " + std::to_string(m_data_size) +
            ", m_position: " + std::to_string(m_position));
    }
    size_t node_pos    = m_position % m_node_size; // 在当前node中的位置
    size_t node_remain = m_cur->size - node_pos;   // 当前node剩余大小
    size_t buf_pos     = 0;                        // 当前buf读到的位置

    while (size > 0)
    {
        if (size <= node_remain)
        {
            memcpy((char *)buf + buf_pos, m_cur->data + node_pos, size);
            if (m_cur->size == (node_pos + size))
            {
                m_cur = m_cur->next;
            }
            m_position += size;

            buf_pos += size;
            size = 0;
        }
        else
        {
            memcpy((char *)buf + buf_pos, m_cur->data + node_pos, node_remain);
            m_position += node_remain;
            buf_pos += node_remain;
            size -= node_remain;
            m_cur       = m_cur->next;
            node_remain = m_cur->size;
            node_pos    = 0;
        }
    }
}

void ByteArray::read(void *buf, size_t size, size_t position) const
{
    if (size == 0)
    {
        return;
    }
    if (size > (m_data_size - position))
    {
        throw std::runtime_error(
            "ByteArray::read: read out of range, m_data_size: " + std::to_string(m_data_size) +
            ", m_position: " + std::to_string(m_position));
    }
    size_t node_pos    = position % m_node_size; // 在当前node中的位置
    size_t node_remain = m_cur->size - node_pos; // 当前node剩余大小
    size_t buf_pos     = 0;                      // 当前buf读到的位置
    auto cur           = m_cur;

    while (size > 0)
    {
        if (size <= node_remain)
        {
            memcpy((char *)buf + buf_pos, cur->data + node_pos, size);
            if (cur->size == (node_pos + size))
            {
                cur = cur->next;
            }
            position += size;

            buf_pos += size;
            size = 0;
        }
        else
        {
            memcpy((char *)buf + buf_pos, cur->data + node_pos, node_remain);
            position += node_remain;
            buf_pos += node_remain;
            size -= node_remain;
            cur         = cur->next;
            node_remain = cur->size;
            node_pos    = 0;
        }
    }
}

size_t ByteArray::getPosition() { return m_position; }

void ByteArray::setPosition(size_t pos)
{
    if (pos > m_total_size)
    {
        throw std::runtime_error("ByteArray::setPosition: pos out of range, m_data_size: " +
                                 std::to_string(m_data_size) + ", pos: " + std::to_string(pos));
    }
    m_position = pos;
    m_cur      = m_root;
    if (m_position > m_data_size)
    {
        m_data_size = m_position;
    }
    while (pos > m_cur->size)
    {
        m_cur = m_cur->next;
        pos -= m_cur->size;
    }
    if (pos == m_cur->size)
    {
        m_cur = m_cur->next;
    }
}

size_t ByteArray::getNodeSize() { return m_node_size; }

size_t ByteArray::getReadSize() { return m_data_size - m_position; }

size_t ByteArray::count() const
{
    return m_data_size / m_node_size + (m_data_size % m_node_size > 0 ? 1 : 0);
}

size_t ByteArray::size() const { return m_data_size; }

bool ByteArray::isLittleEndian() { return m_ba_endian == LON_LITTLE_ENDIAN; }

void ByteArray::setLittleEndian(bool is_little)
{
    is_little ? m_ba_endian = LON_LITTLE_ENDIAN : m_ba_endian = LON_BIG_ENDIAN;
}

bool ByteArray::writeToFile(const std::string &path)
{
    std::ofstream out;
    out.open(path, std::ios::trunc | std::ios::binary);
    if (!out.is_open())
    {
        throw std::runtime_error("ByteArray::writeToFile: open file failed, path: " + path);
        return false;
    }
    size_t read_size = getReadSize();
    size_t pos       = m_position;
    auto cur         = m_cur;

    while (read_size > 0)
    {
        size_t node_pos = pos % m_node_size;
        int64_t len     = (read_size > (int64_t)m_node_size ? m_node_size : read_size) - node_pos;
        out.write(cur->data + node_pos, len);
        cur = cur->next;
        pos += len;
        read_size -= len;
    }
    return true;
}

bool ByteArray::readFromFile(const std::string &path)
{
    std::ifstream in;
    in.open(path, std::ios::binary);
    if (!in.is_open())
    {
        throw std::runtime_error("ByteArray::readFromFile: open file failed, path: " + path);
        return false;
    }
    std::shared_ptr<char> buf(new char[m_node_size], [](char *p) { delete[] p; });
    while (!in.eof())
    {
        in.read(buf.get(), m_node_size);
        write(buf.get(), in.gcount());
    }

    return true;
}

std::string ByteArray::toString() const
{
    std::string str;
    str.resize(m_data_size - m_position);
    if (str.empty())
    {
        return str;
    }
    read(&str[0], str.size(), m_position);
    return str;
}

std::string ByteArray::toStringHex() const
{
    std::stringstream ss;
    std::string str = toString();
    for (size_t cnt = 0; cnt < str.size(); ++cnt)
    {
        if (cnt > 0 && cnt % 32 == 0)
        {
            ss << std::endl;
        }
        ss << std::setw(2) << std::setfill('0') << std::hex << (int)(uint8_t)str[cnt] << " ";
    }
    return ss.str();
}

size_t ByteArray::getReadBuffers(std::vector<iovec> &bufs, size_t len) const
{
    len = len > (m_data_size - m_position) ? (m_data_size - m_position) : len;
    if (len == 0)
    {
        return 0;
    }

    size_t size        = len;
    size_t node_pos    = m_position % m_node_size;
    size_t node_remain = m_cur->size - node_pos;
    struct iovec iov;
    auto cur = m_cur;

    while (len > 0)
    {
        if (node_remain >= len)
        {
            iov.iov_base = cur->data + node_pos;
            iov.iov_len  = len;
            len          = 0;
        }
        else
        {
            iov.iov_base = cur->data + node_pos;
            iov.iov_len  = node_remain;
            len -= node_remain;
            cur         = cur->next;
            node_remain = cur->size;
            node_pos    = 0;
        }
        bufs.push_back(iov);
    }
    return size;
}

size_t ByteArray::getReadBuffers(std::vector<iovec> &bufs, size_t len, size_t position) const
{
    len = len > (m_data_size - position) ? (m_data_size - position) : len;
    if (len == 0)
    {
        return 0;
    }

    size_t size        = len;
    size_t node_pos    = position % m_node_size;
    size_t node_remain = m_cur->size - node_pos;
    struct iovec iov;
    auto cur     = m_root;
    size_t count = position / m_node_size;
    while (count > 0)
    {
        cur = cur->next;
        --count;
    }

    while (len > 0)
    {
        if (node_remain >= len)
        {
            iov.iov_base = cur->data + node_pos;
            iov.iov_len  = len;
            len          = 0;
        }
        else
        {
            iov.iov_base = cur->data + node_pos;
            iov.iov_len  = node_remain;
            len -= node_remain;
            cur         = cur->next;
            node_remain = cur->size;
            node_pos    = 0;
        }
        bufs.push_back(iov);
    }
    return size;
}

size_t ByteArray::getWriteBuffers(std::vector<iovec> &bufs, size_t len)
{
    if (len == 0)
    {
        return 0;
    }
    addTotalSize(len);
    size_t size        = len;
    size_t node_pos    = m_position % m_node_size;
    size_t node_remain = m_cur->size - node_pos;
    struct iovec iov;
    auto cur = m_cur;

    while (len > 0)
    {
        if (node_remain >= len)
        {
            iov.iov_base = cur->data + node_pos;
            iov.iov_len  = len;
            len          = 0;
        }
        else
        {
            iov.iov_base = cur->data + node_pos;
            iov.iov_len  = node_remain;
            len -= node_remain;
            cur         = cur->next;
            node_remain = cur->size;
            node_pos    = 0;
        }
        bufs.push_back(iov);
    }
    return size;
}

void ByteArray::addTotalSize(size_t size)
{
    if (size == 0)
    {
        return;
    }
    size_t old_remain_size = getRemainSize();
    if (old_remain_size >= size)
    {
        return;
    }
    size = size - old_remain_size;
    // size_t new_node_count = (size / m_node_size) + (size % m_node_size < old_remain_size ? 1 :
    // 0);
    // 向上取整
    size_t new_node_count = ceil(size * 1.0f / m_node_size);
    auto last_node        = m_root;
    while (last_node->next != nullptr)
    {
        last_node = last_node->next;
    }
    ByteArrayNode *new_node_first = nullptr;
    for (size_t i = 0; i < new_node_count; ++i)
    {
        auto new_node = new ByteArrayNode(m_node_size);
        if (!new_node_first)
        {
            new_node_first = new_node;
        }
        last_node->next = new_node;
        m_total_size += m_node_size;
        last_node = last_node->next;
    }
    if (old_remain_size == 0)
    {
        m_cur = new_node_first;
    }
}

size_t ByteArray::getRemainSize() { return m_total_size - m_position; }

} // namespace util
} // namespace lon