#include "graph\vector.h"
#include "service\sse.h"
#include <random>

ARBOR_BEGIN

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
__m128 __vectorcall randomVector(_In_ const float a)
{
    static std::random_device rd {};
    static std::mt19937 engine {rd()};
    static std::uniform_real_distribution<float> distribution {0.0f, 1.0f};
    sse_t value = {distribution(engine), 0.5f, distribution(engine), 0.5f};
    __m128 temp = _mm_load_ps(value.data);
    temp = _mm_hsub_ps(temp, temp);
    value.data[0] = a * 2;
    __m128 temp2 = _mm_load_ps(value.data);
    temp2 = _mm_shuffle_ps(temp2, temp2, 0);
    return _mm_mul_ps(temp, temp2);
}

ARBOR_END
