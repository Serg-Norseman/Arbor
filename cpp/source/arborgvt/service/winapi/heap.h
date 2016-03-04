#pragma once
#include "ns\wapi.h"
#include "service\winapi\uh.h"
#include <Windows.h>

WAPI_BEGIN

/**
 * Type traits class for 'HANDLE to heap' type.
 */
class heap_traits: public unique_handle_traits<HANDLE>
{
public:
    static void close(_In_ const type handle) noexcept
    {
        HeapDestroy(handle);
    }
};

typedef unique_handle<heap_traits> heap_t;

WAPI_END
