﻿#pragma once
#include "graph/vector.h"
#include "ns/arbor.h"
#include "service/sse.h"
#include "service/stladdon.h"
#include <d2d1.h>

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
    vertex() = default;

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
        m_fixed {false}
    {
        sse_t value;
        /*
         * Set `m_coordinates` to QNaN.
         *
         * I use QNaN (as .NET Framework 4.6 does (remember about roots of the code)); therefore the code mostly doesn't
         * throw an exception. But if one turns to SNaN, the code may throw an exception on SSE instructions once a
         * floating-point exception was unmasked.
         *
         * By default, the C++ run-time libraries mask all floating-point exceptions. And I don't change it.
         *
         * I believe this all is totally unnecessary -- initializing `m_coordinates` with NaN. Code never uses this
         * ctor but always use another one, which initialize `m_coordinates` with a specific floating-point value.
         * Having this ctor available just forces code to do additinal checks before use vertex coordinates, just like
         * this:
         *
         * if (0b0011 == (0b0011 & _mm_movemask_ps(_mm_cmpeq_ps(m_coordinate, m_coordinate))))
         * { ... }
         *
         * Both `CMPPS` and `MOVMSKPS` are far from being the fastest instructions.
         */
        *(reinterpret_cast<uint32_t*> (value.data)) = 0x7FC00000;
        m_coordinates = _mm_load_ps(value.data);
        m_coordinates = _mm_shuffle_ps(m_coordinates, m_coordinates, 0);
        value.data[0] = 1.0f;
        m_mass = _mm_load_ps(value.data);
        m_mass = _mm_shuffle_ps(m_mass, m_mass, 0);
        // Set `m_force` and `m_velocity` to zero.
        m_force = getZeroVector();
        m_velocity = m_force;
        // Set `m_color` to gray.
        value = {0.501960814f, 0.501960814f, 0.501960814f, 1.0f};
        m_color = _mm_load_ps(value.data);
        // Set `m_textColor` to `COLOR_WINDOWTEXT`.
        D2D1_COLOR_F color = D2D1::ColorF {GetSysColor(COLOR_WINDOWTEXT), 1.0f};
        value = {color.r, color.g, color.b, color.a};
        m_textColor = _mm_load_ps(value.data);
    }

    /*
     * VC++ 2015 Update 1, assigning a `__m128` variable to another `__m128` variable: when the VC++ compiler makes
     * "debug" build (x86 or x64) the compiler generates the `MOVUPS` instruction (for my current Debug settings).
     * Although all the affected memory location are aligned on a 16-byte boundary. But when CL makes "release" build
     * (x86 or x64), it generates the `MOVAPS` instruction (one that I need). So it ain't required to use `_mm_load_ps`
     * and `_mm_store_ps` intrinsics.
     *
     * Moreover, sometime even Debug builds use `MOVAPS` instruction.
     */
    vertex(_In_ STLADD string_type&& name, _In_ __m128 coordinates)
        :
        vertex(std::move(name))
    {
        m_coordinates = coordinates;
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
        // `m_coordinates` can be NaN, but `XORPS` doesn't fail on NaNs.
        // 'Cos this method uses XOR-swapping never call it to swap an object with itself.
        m_coordinates = _mm_xor_ps(m_coordinates, right.m_coordinates);
        right.m_coordinates = _mm_xor_ps(m_coordinates, right.m_coordinates);
        m_coordinates = _mm_xor_ps(m_coordinates, right.m_coordinates);
        m_color = _mm_xor_ps(m_color, right.m_color);
        right.m_color = _mm_xor_ps(m_color, right.m_color);
        m_color = _mm_xor_ps(m_color, right.m_color);
        m_textColor = _mm_xor_ps(m_textColor, right.m_textColor);
        right.m_textColor = _mm_xor_ps(m_textColor, right.m_textColor);
        m_textColor = _mm_xor_ps(m_textColor, right.m_textColor);
        m_mass = _mm_xor_ps(m_mass, right.m_mass);
        right.m_mass = _mm_xor_ps(m_mass, right.m_mass);
        m_mass = _mm_xor_ps(m_mass, right.m_mass);
        m_force = _mm_xor_ps(m_force, right.m_force);
        right.m_force = _mm_xor_ps(m_force, right.m_force);
        m_force = _mm_xor_ps(m_force, right.m_force);
        m_velocity = _mm_xor_ps(m_velocity, right.m_velocity);
        right.m_velocity = _mm_xor_ps(m_velocity, right.m_velocity);
        m_velocity = _mm_xor_ps(m_velocity, right.m_velocity);
        std::swap(m_name, right.m_name);
        std::swap(m_data, right.m_data);
        std::swap(m_fixed, right.m_fixed);
    }

    __m128 __vectorcall getCoordinates() const noexcept
    {
        return m_coordinates;
    }

    void __vectorcall setCoordinates(_In_ const __m128 value) noexcept
    {
        m_coordinates = value;
    }

    __m128 __vectorcall getColor() const noexcept
    {
        return m_color;
    }

    void __vectorcall setColor(_In_ __m128 value) noexcept
    {
        m_color = value;
    }

    __m128 __vectorcall getTextColor() const noexcept
    {
        return m_textColor;
    }

    void __vectorcall setTextColor(_In_ __m128 value) noexcept
    {
        m_textColor = value;
    }

    __m128 __vectorcall getMass() const noexcept
    {
        return m_mass;
    }

    void __vectorcall setMass(_In_ __m128 value) noexcept
    {
        m_mass = value;
    }

    __m128 __vectorcall getForce() const noexcept
    {
        return m_force;
    }

    void __vectorcall setForce(_In_ const __m128 value) noexcept
    {
        m_force = value;
    }

    __m128 __vectorcall getVelocity() const noexcept
    {
        return m_velocity;
    }

    void __vectorcall setVelocity(_In_ const __m128 value) noexcept
    {
        m_velocity = value;
    }

    const STLADD string_type* getName() const noexcept
    {
        return &m_name;
    }

    void* getData() const noexcept
    {
        return m_data;
    }

    void setData(_In_ void* value) noexcept
    {
        m_data = value;
    }

    bool getFixed() const noexcept
    {
        return m_fixed;
    }

    void setFixed(_In_ bool value) noexcept
    {
        m_fixed = value;
    }

    void __vectorcall applyForce(_In_ const __m128 value) noexcept
    {
        // `value` must not be zero.
        m_force = _mm_add_ps(m_force, _mm_mul_ps(value, _mm_rcp_ps(m_mass)));
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
    __m128 m_textColor;
    __m128 m_mass;
    __m128 m_force;
    __m128 m_velocity;
    STLADD string_type m_name;
    void* m_data;
    bool m_fixed;
};

ARBOR_END

inline void swap(_Inout_ ARBOR vertex& left, _Inout_ ARBOR vertex& right)
{
    left.swap(right);
}
