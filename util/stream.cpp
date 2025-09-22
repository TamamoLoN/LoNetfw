#include "util/stream.h"

namespace lon
{
namespace util
{
Stream::Stream::Stream() {}

Stream::~Stream() {}

size_t Stream::readF(void *buf, size_t len)
{
    size_t offset = 0;
    size_t left   = len;
    while (left > 0)
    {
        auto length = read((char *)buf + offset, left);
        if (length <= 0)
        {
            return length;
        }
        offset += length;
        left -= length;
    }
    return len;
}

size_t Stream::readF(const ByteArray::Ptr &buf, size_t len)
{
    size_t left = len;
    while (left > 0)
    {
        auto length = read(buf, left);
        if (length <= 0)
        {
            return length;
        }
        left -= length;
    }
    return len;
}

size_t Stream::writeF(const void *buf, size_t len)
{
    size_t offset = 0;
    size_t left   = len;
    while (left > 0)
    {
        auto length = write((const char *)buf + offset, left);
        if (length <= 0)
        {
            return length;
        }
        offset += length;
        left -= length;
    }
    return len;
}

size_t Stream::writeF(const ByteArray::Ptr &buf, size_t len)
{
    size_t left = len;
    while (left > 0)
    {
        auto length = write(buf, left);
        if (length <= 0)
        {
            return length;
        }
        left -= length;
    }
    return len;
}

} // namespace util
} // namespace lon