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
    _In_ D3D_DRIVER_TYPE driverType, _In_ UINT nFlags, _Outptr_result_maybenull_ ID3D11Device** ppDevice)
{
#if defined(_DEBUG)
    nFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    return D3D11CreateDevice(
        nullptr, driverType, nullptr, nFlags, nullptr, 0, D3D11_SDK_VERSION, ppDevice, nullptr, nullptr);
}

template <typename T>
inline HRESULT createD2DFactory(
    _In_ D2D1_FACTORY_TYPE type, _In_ D2D1_FACTORY_OPTIONS& options, _Outptr_result_maybenull_ T** ppFactory)
{
#if defined(_DEBUG)
    options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#else
    options.debugLevel = D2D1_DEBUG_LEVEL_NONE;
#endif
    return D2D1CreateFactory(type, options, ppFactory);
}

inline HRESULT createDWriteFactory(_Outptr_result_maybenull_ IDWriteFactory1** ppFactory)
{
    return
        DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(*ppFactory), reinterpret_cast<IUnknown**> (ppFactory));
}


// Implement acquiring and applying the Direct2D factory lock.
class direct2d_factory_lock
{
public:
    explicit direct2d_factory_lock(_In_ ID2D1Factory1* pFactory)
    {
        WAPI check_hr(pFactory->QueryInterface(m_lock.getAddressOf()));
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
