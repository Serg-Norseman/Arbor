#pragma once
#include "ns\atladd.h"
#include <Windows.h>

ATLADD_BEGIN
template <typename... T>
class __declspec(novtable) implements: public T...
{
public:
    virtual unsigned long __stdcall AddRef() noexcept override
    {
        return InterlockedIncrement(&m_nRefCount);
    }

    virtual unsigned long __stdcall Release() noexcept override
    {
        if (0 == InterlockedDecrement(&m_nRefCount))
        {
            delete this;
            return 0;
        }
        else
        {
            return m_nRefCount;
        }
    }

    virtual HRESULT __stdcall QueryInterface(_In_ const IID& iid, _Deref_out_opt_ void** ppObject) noexcept override
    {
        *ppObject = queryInterface<T...>(iid);
        if (nullptr != *ppObject)
        {
            (static_cast<::IUnknown*> (*ppObject))->AddRef();
            return S_OK;
        }
        else
        {
            return E_NOINTERFACE;
        }
    }


protected:
    implements() /*nothrow*/ = default;
    
    virtual ~implements() //nothrow
    {
    }

    unsigned long m_nRefCount = 1;


private:
    template <typename First, typename... Last>
    void* queryInterface(_In_ const IID& iid) noexcept
    {
        return ((__uuidof(First) == iid) || (__uuidof(::IUnknown) == iid)) ?
            static_cast<First*> (this) :
            findInterface<Last...>(iid);
    }

    template <typename First, typename... Last>
    void* findInterface(_In_ const IID& iid) noexcept
    {
        return (__uuidof(First) == iid) ? static_cast<First*> (this) : findInterface<Last...>(iid);
    }

    template <int = 0>
    void* findInterface(_In_ const IID&) noexcept
    {
        return nullptr;
    }
};
ATLADD_END
