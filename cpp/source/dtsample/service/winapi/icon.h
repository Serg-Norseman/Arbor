#pragma once
#include "ns/wapi.h"
#include "service/winapi/uh.h"
#include <Windows.h>

WAPI_BEGIN

/**
 * Type traits class for the 'HICON' type.
 */
class icon_traits: public unique_handle_traits<HICON>
{
public:
    static void close(_In_ const type handle) noexcept
    {
        DestroyIcon(handle);
    }
};

typedef unique_handle<icon_traits> icon_t;

WAPI_END
