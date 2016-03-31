#pragma once
#include "ns\dxu.h"
#include "service\com\comptr.h"
#include "service\winapi\chkerror.h"
#include "service\functype.h"
#include <d3d11.h>
#include <dcomp.h>
#include <dwrite_2.h>
#include <dxgi1_3.h>
#include <sal.h>

DXU_BEGIN

inline HRESULT createD3Device(
    _In_ D3D_DRIVER_TYPE driverType, _In_ UINT flags, _Outptr_result_maybenull_ ID3D11Device** device)
{
#if defined(_DEBUG)
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    return D3D11CreateDevice(
        nullptr, driverType, nullptr, flags, nullptr, 0, D3D11_SDK_VERSION, device, nullptr, nullptr);
}

template <typename T>
inline HRESULT createD2DFactory(
    _In_ D2D1_FACTORY_TYPE type, _In_ D2D1_FACTORY_OPTIONS& options, _Outptr_result_maybenull_ T** factory)
{
#if defined(_DEBUG)
    options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#else
    options.debugLevel = D2D1_DEBUG_LEVEL_NONE;
#endif
    return D2D1CreateFactory(type, options, factory);
}

inline HRESULT createDWriteFactory(_Outptr_result_maybenull_ IDWriteFactory1** factory)
{
    return DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(*factory), reinterpret_cast<IUnknown**> (factory));
}


// Implement acquiring and applying the Direct2D factory lock.
class direct2d_factory_lock
{
public:
    explicit direct2d_factory_lock(_In_ ID2D1Factory1* factory)
    {
        WAPI check_hr(factory->QueryInterface(m_lock.getAddressOf()));
        m_lock->Enter();
    }

    ~direct2d_factory_lock()
    {
        m_lock->Leave();
    }


private:
    ATLADD com_ptr<ID2D1Multithread> m_lock;
};

DXU_END
