#pragma once
#include <atomic>
#include <vector>
#include <cassert>
template <class T> class spsc_queue
{
  private:
    std::vector<T> m_buffer;
    std::atomic<size_t> m_head;
    std::atomic<size_t> m_tail;

  public:
    spsc_queue(size_t capacity) : m_buffer(capacity + 1), m_head(0), m_tail(0)
    {
    }

    inline bool enqueue(const T &item)
    {
        const size_t tail = m_tail.load(std::memory_order_relaxed);
        const size_t next = (tail + 1) % m_buffer.size();

        if (next == m_head.load(std::memory_order_acquire))
            return false;

        m_buffer[tail] = item;
        m_tail.store(next, std::memory_order_release);
        return true;
    }

    inline bool dequeue(T &item)
    {
        const size_t head = m_head.load(std::memory_order_relaxed);
        
        if (head == m_tail.load(std::memory_order_acquire))
            return false;

        item = m_buffer[head];
        const size_t next = (head + 1) % m_buffer.size();
        m_head.store(next, std::memory_order_release);
        return true;
    }

    inline bool empty() const
    {
        return m_head.load(std::memory_order_relaxed) == m_tail.load(std::memory_order_relaxed);
    }
};