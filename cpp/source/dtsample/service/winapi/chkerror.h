#pragma once
#include "ns/wapi.h"
#include <winerror.h>

WAPI_BEGIN

class check_failed
{
public:
    explicit check_failed(_In_ const long result)
        :
        m_nError {result}
    {
    }


private:
    long m_nError;
};


template<typename T>
inline void check_bool(_In_ const T result)
{
    if (!result)
    {
        throw check_failed {::GetLastError()};
    }
}

inline void check_hr(_In_ const HRESULT result)
{
    if (FAILED(result))
    {
        throw check_failed {result};
    }
}

template<typename T>
inline void check(_In_ const T expected, _In_ const T actual)
{
    if (actual != expected)
    {
        throw check_failed {0};
    }
}

WAPI_END
