#pragma once
#include <intrin.h>

typedef struct alignas(16)
{
    float data[4];
}
sse_t;

class simd_cpu_capabilities
{
public:
    static bool sse41() noexcept
    {
        int info[4];
        __cpuid(info, 0x00);
        if (0 < info[0])
        {
            __cpuid(info, 0x01);
            return 0 != (0x80000 & info[2]);
        }
        else
        {
            return false;
        }
    }
};
