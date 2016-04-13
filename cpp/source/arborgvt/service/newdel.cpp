#include "service/stladdon.h"

_Ret_notnull_ _Post_writable_byte_size_(size)
void* operator new(size_t size)
{
    STLADD default_allocator<unsigned char> allocator {};
    return allocator.allocate(size);
}

void operator delete(void* p)
{
    STLADD default_allocator<unsigned char> allocator {};
    allocator.deallocate(p);
}
