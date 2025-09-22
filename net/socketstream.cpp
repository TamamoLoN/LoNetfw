#include "net/socketstream.h"

namespace lon
{
namespace net
{
SocketStream::SocketStream(const Socket::Ptr &socket, bool proxy) : m_socket(socket), m_proxy(proxy)
{
}

SocketStream::~SocketStream()
{
    if (m_proxy && m_socket)
    {
        close();
    }
}

size_t SocketStream::read(void *buf, size_t len)
{
    if (!isConnected())
    {
        return -1;
    }
    return m_socket->recv(buf, len);
}

size_t SocketStream::read(const util::ByteArray::Ptr &buf, size_t len)
{
    if (!isConnected())
    {
        return -1;
    }
    std::vector<iovec> iovecs = {};
    buf->getWriteBuffers(iovecs, len);
    auto ret = m_socket->recv(&iovecs[0], iovecs.size());
    if (ret > 0)
    {
        buf->setPosition(buf->getPosition() + ret);
    }
    return ret;
}

size_t SocketStream::write(const void *buf, size_t len)
{
    if (!isConnected())
    {
        return -1;
    }
    return m_socket->send(buf, len);
}

size_t SocketStream::write(const util::ByteArray::Ptr &buf, size_t len)
{
    if (!isConnected())
    {
        return -1;
    }
    std::vector<iovec> iovecs = {};
    buf->getReadBuffers(iovecs, len);
    auto ret = m_socket->send(&iovecs[0], iovecs.size());
    if (ret > 0)
    {
        buf->setPosition(buf->getPosition() + ret);
    }
    return ret;
}

void SocketStream::close()
{
    if (m_socket)
    {
        m_socket->close();
    }
}

Socket::Ptr SocketStream::getSocket() const { return m_socket; }

bool SocketStream::isConnected() const { return m_socket && m_socket->isConnected(); }

} // namespace net
} // namespace lon