#pragma once
#include <cstddef>
#include <cstdint>

inline constexpr size_t MEGA = (1024 * 1024);

class ArenaAllocator
{
private:
    size_t m_size;
    std::byte *m_buff;
    std::byte *m_offset;

public:
    inline explicit ArenaAllocator(size_t bytes)
    {
        m_size = bytes;
        m_offset = m_buff = static_cast<std::byte *>(::operator new(m_size));
    }

    ArenaAllocator(const ArenaAllocator &other) = delete;
    ArenaAllocator &operator=(const ArenaAllocator &other) = delete;

    inline ~ArenaAllocator()
    {
        ::operator delete(m_buff);
    }

    template <typename T>
    T *allocate()
    {
        size_t size = sizeof(T);
        size_t alignment = alignof(T);

        uintptr_t address = reinterpret_cast<uintptr_t>(m_offset);

        size_t padding = (alignment - (address % alignment)) % alignment;

        address += padding;

        if (m_offset + padding + size > m_buff + m_size)
            return nullptr;

        m_offset += padding + size;

        return reinterpret_cast<T *>(address);
    }

    void reset()
    {
        m_offset = m_buff;
    }

    size_t size() const
    {
        return m_size;
    }
};