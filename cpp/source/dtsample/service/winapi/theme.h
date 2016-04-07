#pragma once
#include "ns\wapi.h"
#include "service\winapi\uh.h"
#include <Windows.h>
#include <Uxtheme.h>

WAPI_BEGIN

/**
 * Type traits class for the 'HTHEME' type.
 */
class theme_traits: public unique_handle_traits<HTHEME>
{
public:
    static void close(_In_ const type handle) noexcept
    {
        CloseThemeData(handle);
    }
};

typedef unique_handle<theme_traits> theme_t;

WAPI_END
