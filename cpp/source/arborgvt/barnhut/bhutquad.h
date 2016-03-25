#pragma once
#include "graph\vector.h"
#include "ns\barnhut.h"
#include "service\stladdon.h"
#include <deque>
#include <memory>
#include <vector>

BHUT_BEGIN

/**
 * `quad_element` class represents an element stored in Barnes Hut simulation's quad.
 *
 * Remarks:
 * An instance of the `quad_element` class MUST be aligned on a 16-byte boundary! See comments for `vertex` class in
 * vertex.h file.
 *
 * `quad_element` default constructor initializes element's coordinates with zeros, not with NaNs as original C# code
 * does. Its derived `particle` class declared this inherited default ctor as deleted, therefore caller has to assign
 * some value to element's coordinates (may be NaNs). Code will check NaNs in particles. But `branch` derived class does
 * exploits the zeroed vector.
 */
class particle;
class branch;
class quad_element abstract
{
public:
    typedef enum
    {
        NorthEastQuad,
        NorthWestQuad,
        SouthEastQuad,
        SouthWestQuad,
        UnknownQuad
    }
    quad_index;

    typedef std::deque<std::unique_ptr<particle>, STLADD default_allocator<std::unique_ptr<particle>>> particles_cont_t;

    quad_element() noexcept
        :
#if defined(__ICL)
        m_coordinates(ARBOR getZeroVector()),
#else
        m_coordinates {ARBOR getZeroVector()},
#endif
        m_mass {0.0f}
    {
    }

    quad_element(_In_ const quad_element&) = delete;

    quad_element(_In_ quad_element&& right) noexcept
    {
        if (this != &right)
        {
            right.swap(*this);
        }
    }

    quad_element(_In_ __m128 coordinates, _In_ const float mass) noexcept
        :
#if defined(__ICL)
        m_coordinates(coordinates),
#else
        m_coordinates {coordinates},
#endif
        m_mass {mass}
    {
    }

    virtual ~quad_element() = default;

    quad_element& operator =(_In_ const quad_element&) = delete;

    quad_element& operator =(_In_ quad_element&& right) noexcept
    {
        if (this != &right)
        {
            right.swap(*this);
        }
        return *this;
    }

    static void* operator new(_In_ const size_t size)
    {
        STLADD aligned_sse_allocator<quad_element> allocator {};
        return allocator.allocate(size, 0);
    }

    static void operator delete(_In_ void* p)
    {
        STLADD aligned_sse_allocator<quad_element> allocator {};
        allocator.deallocate(p);
    }

    void swap(_Inout_ quad_element& right) noexcept
    {
        // `m_coordinates` can be NaN, but `XORPS` doesn't fail on NaNs.
        // 'Cos this method uses XOR-swapping never call it to swap an object with itself.
        m_coordinates = _mm_xor_ps(m_coordinates, right.m_coordinates);
        right.m_coordinates = _mm_xor_ps(m_coordinates, right.m_coordinates);
        m_coordinates = _mm_xor_ps(m_coordinates, right.m_coordinates);
        std::swap(m_mass, right.m_mass);
    }

    virtual branch* __fastcall handleParticle(
        _In_ const particle* p, _In_ branch* b, _In_ quad_index quad, _In_ particles_cont_t* particles) = 0;

    __m128 __vectorcall getCoordinates() const noexcept
    {
        return m_coordinates;
    }

    void __vectorcall setCoordinates(_In_ __m128 value) noexcept
    {
        m_coordinates = value;
    }

    float getMass() const noexcept
    {
        return m_mass;
    }

    void setMass(_In_ float value) noexcept
    {
        m_mass = value;
    }


protected:
    __m128 m_coordinates;
    float m_mass;
};


class branch final: public quad_element
{
public:
    branch(_In_ const branch&) = delete;

    branch(_In_ branch&& right) noexcept
    {
        if (this != &right)
        {
            right.swap(*this);
        }
    }

    branch(_In_ __m128 area) noexcept
        :
#if defined(__ICL)
        m_area(area),
#else
        m_area {area},
#endif
        m_quads {4}
    {
    }

    branch& operator =(_In_ const branch&) = delete;

    branch& operator =(_In_ branch&& right) noexcept
    {
        if (this != &right)
        {
            right.swap(*this);
        }
        return *this;
    }

    void swap(_Inout_ branch& right) noexcept
    {
        base_class_t::swap(right);
        m_area = _mm_xor_ps(m_area, right.m_area);
        right.m_area = _mm_xor_ps(m_area, right.m_area);
        m_area = _mm_xor_ps(m_area, right.m_area);
        std::swap(m_quads, right.m_quads);
    }

    virtual branch* __fastcall handleParticle(
        _In_ const particle* p, _In_ branch* b, _In_ quad_index, _In_ particles_cont_t*) override;

    quad_index __fastcall getQuad(_In_ const particle* p) const noexcept;
    _Check_return_ bool __fastcall getQuadContent(
        _In_ quad_index quad, _Outptr_result_maybenull_ quad_element** element) const noexcept;

    void __fastcall setQuadContent(_In_ quad_index quad, _In_ std::unique_ptr<quad_element>&& element) noexcept
    {
        if (UnknownQuad != quad)
        {
            m_quads.at(quad) = std::move(element);
        }
    }

    __m128 __vectorcall getArea() const noexcept
    {
        return m_area;
    }

    void increaseParameters(_In_ const particle* p) noexcept;


private:
    typedef quad_element base_class_t;
    typedef std::vector<std::unique_ptr<quad_element>, STLADD default_allocator<std::unique_ptr<quad_element>>>
        quads_cont_t;

    // Quad bound. The vectors formatted as [top-y, bottom-y, left-x, right-x].
    __m128 m_area;
    quads_cont_t m_quads;
};


class particle final: public quad_element
{
public:
    using quad_element::quad_element;

    particle() = delete;

    virtual branch* __fastcall handleParticle(
        _In_ const particle* p, _In_ branch* b, _In_ quad_index quad, _In_ particles_cont_t* particles) override;


private:
    typedef quad_element base_class_t;
};

BHUT_END

inline void swap(_Inout_ BHUT branch& left, _Inout_ BHUT branch& right)
{
    left.swap(right);
}

inline void swap(_Inout_ BHUT particle& left, _Inout_ BHUT particle& right)
{
    left.swap(right);
}
