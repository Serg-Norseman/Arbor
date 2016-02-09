#pragma once
#include "ns\arbor.h"
#include "service\ssetype.h"
#include "service\stladdon.h"

ARBOR_BEGIN

/**
 * `vertex` class represents a graph vertex.
 *
 * Remarks:
 * An instance of the `vertex` class MUST be aligned on a 16-byte boundary!
 */
class vertex
{
public:
    vertex() = delete;
    vertex(_In_ const vertex&) = delete;

    vertex(_In_ vertex&& right) noexcept
    {
        if (this != &right)
        {
            right.swap(*this);
        }
    }

    explicit vertex(_In_ STLADD string_type&& name)
        :
        m_name {std::move(name)},
        m_data {nullptr},
        m_mass {1.0f},
        m_fixed {false}
    {
        sse_t value;
        // Set `m_coordinates` to QNaN.
        *(reinterpret_cast<uint32_t*> (value.data)) = 0x7FC00000;
        m_coordinates = _mm_load_ps(value.data);
        m_coordinates = _mm_shuffle_ps(m_coordinates, m_coordinates, 0);
        // Set `m_force` and `m_velocity` to zero.
        value.data[0] = 0;
        m_force = _mm_load_ps(value.data);
        m_force = _mm_shuffle_ps(m_force, m_force, 0);
        m_velocity = _mm_load_ps(value.data);
        m_velocity = _mm_shuffle_ps(m_velocity, m_velocity, 0);
        // Set `m_color` to gray.
        value = {0.501960814f, 0.501960814f, 0.501960814f, 1.0f};
        m_color = _mm_load_ps(value.data);
    }

    /*
     * Following DirectXMath recommendations I don't use `__vectorcall` calling convention for the ctor.
     *
     * VC++ 2015 Update 1, assigning a `__m128` variable to another `__m128` variable: when the VC++ compiler makes
     * "debug" build (x86 or x64) the compiler generates the `MOVUPS` instruction (for my current Debug settings).
     * Although all the affected memory location are aligned on a 16-byte boundary. But when CL makes "release" build
     * (x86 or x64), it generates the `MOVAPS` instruction (one that I need). So it ain't required to use `_mm_load_ps`
     * and `_mm_store_ps` intrinsics.
     */
    vertex(_In_ STLADD string_type&& name, _In_ __m128 coordinates)
        :
        m_coordinates {coordinates},
        m_name {std::move(name)},
        m_data {nullptr},
        m_mass {1.0f},
        m_fixed {false}
    {
        sse_t value;
        // Set `m_force` and `m_velocity` to zero.
        value.data[0] = 0;
        m_force = _mm_load_ps(value.data);
        m_force = _mm_shuffle_ps(m_force, m_force, 0);
        m_velocity = _mm_load_ps(value.data);
        m_velocity = _mm_shuffle_ps(m_velocity, m_velocity, 0);
        // Set `m_color` to gray.
        value = {0.501960814f, 0.501960814f, 0.501960814f, 1.0f};
        m_color = _mm_load_ps(value.data);
    }

    vertex& operator =(_In_ const vertex&) = delete;

    vertex& operator =(_In_ vertex&& right) noexcept
    {
        if (this != &right)
        {
            right.swap(*this);
        }
        return *this;
    }

    void swap(_Inout_ vertex& right) noexcept
    {
        // `m_coordinates` can be NaN -- check it.
        __m128 cmp = _mm_cmpeq_ps(m_coordinates, m_coordinates);
        bool leftIsNaN = !_mm_movemask_ps(cmp);
        cmp = _mm_cmpeq_ps(right.m_coordinates, right.m_coordinates);
        bool rightIsNaN = !_mm_movemask_ps(cmp);
        if (!leftIsNaN || !rightIsNaN)
        {
            cmp = _mm_cmpeq_ps(m_coordinates, right.m_coordinates);
            if (0x0F != _mm_movemask_ps(cmp))
            {
                // Make swap w/o temporary storage using `XORPS`. The latter itself doesn't fail on NaNs but
                // XOR-swapping of two equal value always gives zero.
                cmp = _mm_xor_ps(m_coordinates, right.m_coordinates);
                right.m_coordinates = _mm_xor_ps(cmp, right.m_coordinates);
                m_coordinates = _mm_xor_ps(cmp, right.m_coordinates);
            }
        }
        cmp = _mm_xor_ps(m_color, right.m_color);
        right.m_color = _mm_xor_ps(cmp, right.m_color);
        m_color = _mm_xor_ps(cmp, right.m_color);
        cmp = _mm_xor_ps(m_force, right.m_force);
        right.m_force = _mm_xor_ps(cmp, right.m_force);
        m_force = _mm_xor_ps(cmp, right.m_force);
        cmp = _mm_xor_ps(m_velocity, right.m_velocity);
        right.m_velocity = _mm_xor_ps(cmp, right.m_velocity);
        m_velocity = _mm_xor_ps(cmp, right.m_velocity);
        std::swap(m_name, right.m_name);
        std::swap(m_data, right.m_data);
        std::swap(m_mass, right.m_mass);
        std::swap(m_fixed, right.m_fixed);
    }

    __m128 __vectorcall getCoordinates() const
    {
        return m_coordinates;
    }

    void __vectorcall setCoordinates(_In_ const __m128 value)
    {
        m_coordinates = value;
    }

    void __vectorcall applyForce(_In_ const __m128 value)
    {
        sse_t massData;
        massData.data[0] = m_mass;
        __m128 mass = _mm_load_ps(massData.data);
        mass = _mm_shuffle_ps(mass, mass, 0);
        // Try to replace `_mm_div_ps` with `_mm_rcp_ps` "+" `_mm_mul_ps`?
        m_force = _mm_add_ps(m_force, _mm_div_ps(value, mass));
    }

private:
    /*
     * Instances of the `vertex` class must be aligned on a 16-byte boundary! Because the class contains and uses
     * `__m128` data members.
     *
     * If you're gonna use `vertex` objects within a STL container you HAVE TO write a custom allocator -- STL default
     * allocator ignores aligning of T-type when allocates memory block. Because its `allocate` member function
     * allocates a memory without knowing what C++ type will be construct in that memory. That's why I didn't declare
     * `vertex` class with `__declspec(align(16))` attribute / `alignas(16)` specifier. Anyway the class has it
     * implicitly because of C++ compiler math.
     */
    __m128 m_coordinates;
    __m128 m_color;
    __m128 m_force;
    __m128 m_velocity;
    STLADD string_type m_name;
    void* m_data;
    float m_mass;
    bool m_fixed;
};

ARBOR_END

inline void swap(_Inout_ ARBOR vertex& left, _Inout_ ARBOR vertex& right)
{
    left.swap(right);
}