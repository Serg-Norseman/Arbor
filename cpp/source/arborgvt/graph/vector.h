#pragma once
#include "ns\arbor.h"
#include <sal.h>
#include <xmmintrin.h>

ARBOR_BEGIN

extern __m128 __vectorcall getZeroVector();

extern __m128 __vectorcall randomVector(_In_ const __m128 c);
extern __m128 __vectorcall randomVector(_In_ const float x, _In_ const float y);
extern __m128 __vectorcall randomVector(_In_ const float a);

ARBOR_END
