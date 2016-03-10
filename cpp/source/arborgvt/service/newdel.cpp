#include "service\stladdon.h"

void* operator new(_In_ const size_t size)
{
    STLADD default_allocator<unsigned char> allocator {};
    return allocator.allocate(size);
}

void operator delete(_In_ void* p)
{
    STLADD default_allocator<unsigned char> allocator {};
    allocator.deallocate(p);
}
