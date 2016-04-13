#include "graph/vector.h"
#include "service/sse.h"
#include <random>

ARBOR_BEGIN

static __m128 zero;

class zero_initializer
{
public:
    zero_initializer()
    {
        sse_t value;
        *(reinterpret_cast<uint32_t*> (value.data)) = 0;
        zero = _mm_load_ps(value.data);
        zero = _mm_shuffle_ps(zero, zero, 0);
    }
};
static zero_initializer zero_init {};


/**
 * Gets vector of four packed single precision floating-point zero values.
 *
 * Parameters:
 * None.
 *
 * Returns:
 * `__m128` zeroed value.
 */
__m128 __vectorcall getZeroVector()
{
    return zero;
}


/**
 * Generates new 2D vector with random coordinates.
 *
 * Parameters:
 * >c
 * Coefficient for all vector components.
 *
 * Returns:
 * Vector with random coordinates using the following format:
 *     [y, x, y, x]
 * The result vector has the same the first and the third floating-point values, and the same the second and the fourth
 * values.
 */
__m128 __vectorcall randomVector(_In_ const __m128 c)
{
    static std::random_device rd {};
    static std::mt19937 engine {rd()};
    static std::uniform_real_distribution<float> distribution {0.0f, 1.0f};
    sse_t value = {distribution(engine), 0.5f, distribution(engine), 0.5f};
    __m128 temp = _mm_load_ps(value.data);
    temp = _mm_hsub_ps(temp, temp);
    return _mm_mul_ps(temp, c);
}


/**
 * Generates new 2D vector with random coordinates.
 *
 * Parameters:
 * >x
 * Coefficient for x-component.
 * >y
 * Coefficient for y-component.
 *
 * Returns:
 * Vector with random coordinates using the following format:
 *     [y, x, y, x]
 * The result vector has the same the first and the third floating-point values, and the same the second and the fourth
 * values.
 */
__m128 __vectorcall randomVector(_In_ const float x, _In_ const float y)
{
    sse_t value;
    value.data[0] = x;
    value.data[1] = y;
    __m128 temp = _mm_load_ps(value.data);
    return randomVector(_mm_shuffle_ps(temp, temp, 0b01000100));
}


/**
 * Generates new 2D vector with random coordinates.
 *
 * Parameters:
 * >a
 * Some coefficient.
 *
 * Returns:
 * Vector with random coordinates using the following format:
 *     [y, x, y, x]
 * The result vector has the same the first and the third floating-point values, and the same the second and the fourth
 * values.
 *
 * Remarks:
 * This is `ArborGVT::ArborPoint::newRnd` method translated from the original C# code.
 *
 * `a` is always doubled inside the function, so that's a stuff to be optimized if you can guarantee that you always
 * call this function with immediate argument.
 */
inline __m128 __vectorcall randomVector(_In_ const float a)
{
    return randomVector(a * 2.0f, a * 2.0f);
}

ARBOR_END
