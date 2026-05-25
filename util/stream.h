#pragma once
#include "util/serializer.h"

namespace lon
{
namespace util
{
class LON_API Stream
{
  public:
    using Ptr = std::shared_ptr<Stream>;
    explicit Stream();
    virtual ~Stream();

    virtual ssize_t read(void *buf, size_t len)                 = 0;
    virtual ssize_t read(const ByteArray::Ptr &buf, size_t len) = 0;
    virtual ssize_t readF(void *buf, size_t len);
    virtual ssize_t readF(const ByteArray::Ptr &buf, size_t len);

    virtual ssize_t write(const void *buf, size_t len)           = 0;
    virtual ssize_t write(const ByteArray::Ptr &buf, size_t len) = 0;
    virtual ssize_t writeF(const void *buf, size_t len);
    virtual ssize_t writeF(const ByteArray::Ptr &buf, size_t len);

    virtual void close() = 0;
};
} // namespace util
} // namespace lon