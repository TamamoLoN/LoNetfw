#pragma once

#include "net/socket.h"
#include "util/stream.h"

namespace lon
{
namespace net
{
class LON_API SocketStream : public util::Stream
{
  public:
    using Ptr = std::shared_ptr<SocketStream>;
    /**
     * @param proxy 是否代理，默认为true
     * @param socket 套接字
     * @brief 构造函数
     * @note 默认代理，即使用代理套接字进行读写操作,并且析构函数会关闭套接字
     */
    SocketStream(const Socket::Ptr &socket, bool proxy = true);
    virtual ~SocketStream();

    ssize_t read(void *buf, size_t len) override;
    ssize_t read(const util::ByteArray::Ptr &buf, size_t len) override;

    ssize_t write(const void *buf, size_t len) override;
    ssize_t write(const util::ByteArray::Ptr &buf, size_t len) override;

    void close() override;

    Socket::Ptr getSocket() const;
    bool isConnected() const;
    bool isEof() const;

  protected:
    Socket::Ptr m_socket;
    bool m_proxy;
    bool m_eof;

  private:
};
} // namespace net
} // namespace lon