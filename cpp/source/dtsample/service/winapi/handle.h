#pragma once
#include "ns\wapi.h"
#include "service\winapi\uh.h"
#include <Windows.h>

WAPI_BEGIN

/**
 * Type traits class for the 'HANDLE' type.
 */
class handle_traits: public unique_handle_traits<HANDLE>
{
public:
    static void close(_In_ const type handle) noexcept
    {
        CloseHandle(handle);
    }
};

typedef unique_handle<handle_traits> handle_t;

WAPI_END
