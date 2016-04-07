#pragma once
#include "graph\vertex.h"
#include "ns\arbor.h"
#include "service\stladdon.h"
#include <d2d1.h>

ARBOR_BEGIN

/**
 * `edge` class represents a graph edge.
 *
 * Remarks:
 * An instance of the `edge` class MUST be aligned on a 16-byte boundary! To guarantee this the `edge` class overloads
 * `new` and `delete` operators.
 */
class edge
{
public:
    edge() = delete;
    edge(_In_ const edge&) = delete;

    edge(
        _In_ vertex* tail,
        _In_ vertex* head,
        _In_ const float length,
        _In_ const float stiffness,
        _In_ const bool directed,
        _In_ const D2D1_COLOR_F& color) noexcept
        :
        m_tail {tail},
        m_head {head},
        m_data {nullptr},
        m_length {length},
        m_stiffness {stiffness},
        m_directed {directed}
    {
        sse_t value = {color.r, color.g, color.b, color.a};
        m_color = _mm_load_ps(value.data);
    }

    edge(_In_ vertex* tail,
        _In_ vertex* head,
        _In_ const float length,
        _In_ const float stiffness,
        _In_ const bool directed) noexcept
        :
        edge(tail, head, length, stiffness, directed, D2D1::ColorF {GetSysColor(COLOR_WINDOWTEXT), 1.0f})
    {
    }

    edge& operator =(_In_ const edge&) = delete;

#if defined(ARBORGVT_EXPORTS)
    static void* operator new(_In_ const size_t size)
    {
        STLADD aligned_sse_allocator<edge> allocator {};
        return allocator.allocate(size, 0);
    }

    static void operator delete(_In_ void* p)
    {
        STLADD aligned_sse_allocator<edge> allocator {};
        allocator.deallocate(p);
    }
#endif

    void swap(_Inout_ edge& right) noexcept
    {
        m_color = _mm_xor_ps(m_color, right.m_color);
        right.m_color = _mm_xor_ps(m_color, right.m_color);
        m_color = _mm_xor_ps(m_color, right.m_color);
        std::swap(m_tail, right.m_tail);
        std::swap(m_head, right.m_head);
        std::swap(m_data, right.m_data);
        std::swap(m_length, right.m_length);
        std::swap(m_stiffness, right.m_stiffness);
        std::swap(m_directed, right.m_directed);
    }

    __m128 __vectorcall getColor() const
    {
        return m_color;
    }

    vertex* getTail() noexcept
    {
        return m_tail;
    }

    const vertex* getTail() const noexcept
    {
        return m_tail;
    }

    vertex* getHead() noexcept
    {
        return m_head;
    }

    const vertex* getHead() const noexcept
    {
        return m_head;
    }

    void* getData() const noexcept
    {
        return m_data;
    }

    void setData(_In_ void* value) noexcept
    {
        m_data = value;
    }

    float getLength() const noexcept
    {
        return m_length;
    }

    float getStiffness() const noexcept
    {
        return m_stiffness;
    }

    bool getDirected() const noexcept
    {
        return m_directed;
    }


private:
    __m128 m_color;
    vertex* m_tail;
    vertex* m_head;
    void* m_data;
    float m_length;
    float m_stiffness;
    bool m_directed;
};

ARBOR_END

inline void swap(_Inout_ ARBOR edge& left, _Inout_ ARBOR edge& right)
{
    left.swap(right);
}
