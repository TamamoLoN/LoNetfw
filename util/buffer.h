#pragma once

#include "util/noncopyable.h"
#include <atomic>
#include <memory>
#ifdef _WIN32
#else
#include <sys/eventfd.h>
#include <unistd.h>
#endif
#include <vector>

namespace lon
{
namespace util
{
template <typename T> class Buffer
{
  public:
    using Ptr = std::shared_ptr<Buffer>;
    Buffer()  = default;

    // owning constructor
    Buffer(size_t size)
        : m_owner(new T[size], std::default_delete<T[]>()), m_data(m_owner.get()), m_size(size)
    {
    }
    // zero-copy view constructor
    Buffer(std::shared_ptr<T> owner, T *data, size_t size)
        : m_owner(std::move(owner)), m_data(data), m_size(size)
    {
    }
    const T *data() const { return m_data; }
    T *data() { return m_data; }
    size_t size() const { return m_size; }
    bool empty() const { return m_data == nullptr || m_size == 0; }

  private:
    std::shared_ptr<T> m_owner; // 控制生命周期
    T *m_data     = nullptr;
    size_t m_size = 0;
};

template <typename T> class RingBuffer
{
  public:
    RingBuffer(int capacity = 60)
        : m_capacity(capacity), m_num_datas(0), m_buffer(capacity), m_put_pos(0), m_get_pos(0)
    {
    }

    virtual ~RingBuffer() {}

    bool push(const T &data) { return pushData(std::forward<T>(data)); }

    bool push(T &&data) { return pushData(data); }

    bool pop(T &data)
    {
        if (m_num_datas > 0)
        {
            data = std::move(m_buffer[m_get_pos]);
            add(m_get_pos);
            --m_num_datas;
            return true;
        }

        return false;
    }

    bool full() const { return (m_num_datas == m_capacity); }

    bool empty() const { return (m_num_datas == 0); }

    int size() const { return m_num_datas; }

  private:
    template <typename F> bool pushData(F &&data)
    {
        if (m_num_datas < m_capacity)
        {
            m_buffer[m_put_pos] = std::forward<F>(data);
            add(m_put_pos);
            m_num_datas++;
            return true;
        }

        return false;
    }

    void add(int &pos) { pos = (((pos + 1) == m_capacity) ? 0 : (pos + 1)); }

  private:
    int m_capacity;
    int m_put_pos;
    int m_get_pos;
    std::atomic_int m_num_datas;
    std::vector<T> m_buffer;
};

template <typename T> class EventRingBuffer : public Nonecopyable
{
  public:
    using Ptr = std::shared_ptr<EventRingBuffer>;
    explicit EventRingBuffer(size_t capacity)
        : m_capacity(capacity), m_buf(capacity), m_head(0), m_tail(0)
    {
        m_eventfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if (m_eventfd < 0)
        {
            throw std::runtime_error("create eventfd failed");
        }
    }

    ~EventRingBuffer() { close(m_eventfd); }

    int eventFd() const { return m_eventfd; }

    bool push(T &&v)
    {
        size_t tail = m_tail.load(std::memory_order_relaxed);
        size_t next = (tail + 1) % m_capacity;

        if (next == m_head.load(std::memory_order_acquire))
        {
            return false; // full
        }

        m_buf[tail] = std::move(v);
        m_tail.store(next, std::memory_order_release);

        uint64_t one = 1;
        write(m_eventfd, &one, sizeof(one)); // 通知消费者
        return true;
    }

    bool pop(T &out)
    {
        size_t head = m_head.load(std::memory_order_relaxed);
        if (head == m_tail.load(std::memory_order_acquire))
        {
            return false;
        }

        out = std::move(m_buf[head]);
        m_head.store((head + 1) % m_capacity, std::memory_order_release);
        return true;
    }

  private:
    const size_t m_capacity;
    std::vector<T> m_buf;
    std::atomic<size_t> m_head{0};
    std::atomic<size_t> m_tail{0};
    int m_eventfd;
};
} // namespace util
} // namespace lon
