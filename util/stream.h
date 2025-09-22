#pragma once
#include "util/serializer.h"

namespace lon
{
namespace util
{
class Stream
{
  public:
    using Ptr = std::shared_ptr<Stream>;
    Stream();
    virtual ~Stream();

    virtual size_t read(void *buf, size_t len)                 = 0;
    virtual size_t read(const ByteArray::Ptr &buf, size_t len) = 0;
    virtual size_t readF(void *buf, size_t len);
    virtual size_t readF(const ByteArray::Ptr &buf, size_t len);

    virtual size_t write(const void *buf, size_t len)           = 0;
    virtual size_t write(const ByteArray::Ptr &buf, size_t len) = 0;
    virtual size_t writeF(const void *buf, size_t len);
    virtual size_t writeF(const ByteArray::Ptr &buf, size_t len);

    virtual void close() = 0;
};
} // namespace util
} // namespace lon