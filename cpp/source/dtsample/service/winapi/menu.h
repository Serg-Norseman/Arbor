#pragma once
#include "ns\wapi.h"
#include "service\winapi\uh.h"
#include <Windows.h>

WAPI_BEGIN

/**
 * Type traits class for the 'HMENU' type.
 */
class menu_traits: public unique_handle_traits<HMENU>
{
public:
    static void close(_In_ const type handle) noexcept
    {
        DestroyMenu(handle);
    }
};

typedef unique_handle<menu_traits> menu_t;

WAPI_END
