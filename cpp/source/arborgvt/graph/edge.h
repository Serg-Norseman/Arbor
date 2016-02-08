#pragma once
#include "graph\vertex.h"
#include "ns\arbor.h"

ARBOR_BEGIN

class edge
{
public:
    edge() = delete;
    edge(_In_ const edge&) = delete;

    edge(_In_ const vertex* tail, _In_ const vertex* head, _In_ float length, _In_ float stiffness) noexcept
        :
        m_tail {tail},
        m_head {head},
        m_length {length},
        m_stiffness {stiffness},
        m_directed {false}
    {
    }

    edge(_In_ const vertex* tail,
        _In_ const vertex* head,
        _In_ bool directed,
        _In_ float length,
        _In_ float stiffness) noexcept
        :
        edge(tail, head, length, stiffness)
    {
        m_directed = directed;
    }

    edge& operator =(_In_ const edge&) = delete;

    void swap(_Inout_ edge& right) noexcept
    {
        std::swap(m_tail, right.m_tail);
        std::swap(m_head, right.m_head);
        std::swap(m_length, right.m_length);
        std::swap(m_stiffness, right.m_stiffness);
        std::swap(m_directed, right.m_directed);
    }

    const vertex* getTail() const noexcept
    {
        return m_tail;
    }

    const vertex* getHead() const noexcept
    {
        return m_head;
    }


private:
    const vertex* m_tail;
    const vertex* m_head;
    float m_length;
    float m_stiffness;
    bool m_directed;
};

ARBOR_END

inline void swap(_Inout_ ARBOR edge& left, _Inout_ ARBOR edge& right)
{
    left.swap(right);
}
