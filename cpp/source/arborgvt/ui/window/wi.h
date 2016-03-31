#pragma once
#include "ns\atladd.h"
#include "service\com\comptr.h"
#include "service\winapi\directx\dx.h"
#include "service\winapi\wam\animatio.h"
#include <atlbase.h>
#include <atlwin.h>
#include <memory>
#include <vsstyle.h>
#include <wincodec.h>

ATLADD_BEGIN

#pragma region wi_factories
/**
 * 'wi_factories' class groups some COM factories together (allowing for 'window_impl' template classes to have single
 * instances of factories).
 */
class wi_factories
{
protected:
    HRESULT createFactories()
    {
        HRESULT hr = S_OK;
        if (!m_direct2DFactory)
        {
            D2D1_FACTORY_OPTIONS options {};
            hr = DXU createD2DFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, options, m_direct2DFactory.getAddressOf());
        }
        if (!m_directWriteFactory && SUCCEEDED(hr))
        {
            hr = DXU createDWriteFactory(m_directWriteFactory.getAddressOf());
        }
        if (!m_imagingFactory && SUCCEEDED(hr))
        {
            hr = m_imagingFactory.coCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER);
        }
        return hr;
    }

    void releaseAllFactories()
    {
        m_direct2DFactory.reset();
        m_directWriteFactory.reset();
        m_imagingFactory.reset();
    }

    static ATLADD com_ptr<ID2D1Factory1> m_direct2DFactory;
    static ATLADD com_ptr<IDWriteFactory1> m_directWriteFactory;
    static ATLADD com_ptr<IWICImagingFactory> m_imagingFactory;
};
#pragma endregion wi_factories declaration

#pragma region directx_toolkit
/**
 * 'directx_toolkit' implements some methods directly or not used for DirectX rendering.
 */
class directx_toolkit
{
protected:
    HRESULT getDpiForMonitor(_In_ const HWND hwnd, _Out_ float* dpiX, _Out_ float* dpiY) const;

    template <typename Q>
    constexpr float physicalToLogical(_In_ const Q value, _In_ const float dpi) const
    {
        return value * 96.0f / dpi;
    }

    template <typename Q>
    constexpr float logicalToPhysical(_In_ const Q value, _In_ const float dpi) const
    {
        return value * dpi / 96.0f;
    }

    HRESULT createTextLayoutForBodyTitle(
        _In_ IDWriteFactory* directWriteFactory,
        _In_ const HWND hwnd,
        _In_reads_z_(length) LPCTSTR text,
        _In_ const size_t length,
        _In_ const D2D1_SIZE_F* size,
        _COM_Outptr_result_maybenull_ IDWriteTextLayout** resultTextLayout) const;
    HRESULT createTextLayoutForBodyText(
        _In_ IDWriteFactory* directWriteFactory,
        _In_ const HWND hwnd,
        _In_reads_z_(length) LPCTSTR text,
        _In_ const size_t length,
        _In_ const D2D1_SIZE_F* size,
        _COM_Outptr_result_maybenull_ IDWriteTextLayout** resultTextLayout) const;

    HRESULT createTextFormatForBodyTitle(
        _In_ IDWriteFactory* directWriteFactory,
        _In_ const HWND hwnd,
        _COM_Outptr_result_maybenull_ IDWriteTextFormat** resultTextFormat) const
    {
        return createTextFormat(directWriteFactory, hwnd, TEXT_BODYTITLE, resultTextFormat);
    }

    HRESULT createTextFormatForBodyText(
        _In_ IDWriteFactory* directWriteFactory,
        _In_ const HWND hwnd,
        _COM_Outptr_result_maybenull_ IDWriteTextFormat** resultTextFormat) const
    {
        return createTextFormat(directWriteFactory, hwnd, TEXT_BODYTEXT, resultTextFormat);
    }

    HRESULT loadBitmapFromResource(
        _In_z_ LPCTSTR resourceName,
        _In_z_ LPCTSTR resourceType,
        _In_ IWICImagingFactory* imagingFactory,
        _COM_Outptr_result_maybenull_ IWICFormatConverter** bitmapSource) const;
    HRESULT loadBitmapFromResource(
        _In_z_ LPCTSTR resourceName,
        _In_z_ LPCTSTR resourceType,
        _In_ IWICImagingFactory* imagingFactory,
        _In_ ID2D1DeviceContext* dc,
        _COM_Outptr_result_maybenull_ ID2D1Bitmap1** resultBitmap) const;
    HRESULT loadBitmapFromFile(
        _In_z_ LPCTSTR uri,
        _In_ IWICImagingFactory* imagingFactory,
        _In_ ID2D1DeviceContext* dc,
        _COM_Outptr_result_maybenull_ ID2D1Bitmap1** resultBitmap) const;


private:
    HRESULT createTextLayout(
        _In_ IDWriteFactory* directWriteFactory,
        _In_reads_z_(length) LPCTSTR text,
        _In_ const size_t length,
        _In_ IDWriteTextFormat* textFormat,
        _In_ const D2D1_SIZE_F* size,
        _COM_Outptr_result_maybenull_ IDWriteTextLayout** resultTextLayout) const;
    HRESULT createTextFormat(
        _In_ IDWriteFactory* directWriteFactory,
        _In_ const LOGFONT* logFont,
        _COM_Outptr_result_maybenull_ IDWriteTextFormat** resultTextFormat) const;
    HRESULT createTextFormat(
        _In_ IDWriteFactory* directWriteFactory,
        _In_ const HWND hwnd,
        _In_ const int partId,
        _COM_Outptr_result_maybenull_ IDWriteTextFormat** resultTextFormat) const;
};
#pragma endregion directx_toolkit declaration

#pragma region directx_render
/**
 * 'directx_render' is aware of DirectX rendering on a HWND.
 */
class directx_render abstract: protected directx_toolkit
{
protected:
    virtual void draw() = 0;

    virtual void createDeviceResources()
    {
    }

    virtual void releaseDeviceResources()
    {
    }

    HRESULT createDirect3DDevice()
    {
        HRESULT hr = S_OK;
        if (!m_direct3DDevice)
        {
            hr = DXU createD3Device(
                D3D_DRIVER_TYPE_HARDWARE, D3D11_CREATE_DEVICE_BGRA_SUPPORT, m_direct3DDevice.getAddressOf());
            if (DXGI_ERROR_UNSUPPORTED == hr)
            {
                hr = DXU createD3Device(
                    D3D_DRIVER_TYPE_WARP, D3D11_CREATE_DEVICE_BGRA_SUPPORT, m_direct3DDevice.getAddressOf());
            }
        }
        return hr;
    }

    HRESULT createDirect2DDevice(_In_ ID2D1Factory1* direct2DFactory);

    HRESULT createDevice(_In_ const HWND hwnd, _In_ ID2D1Factory1* direct2DFactory);

    void releaseDevice()
    {
        releaseDeviceResources();
        m_direct2DContext.reset();
        m_direct2DDevice.reset();
        m_swapChain.reset();
        m_direct3DDevice.reset();
    }

    auto isDirect2DContextCreated() const noexcept
    {
        // A convenient way to determine whether DirectX-rendering devices must be recreated.
        return static_cast<bool> (m_direct2DContext);
    }

    void render(_In_ const HWND hwnd, _In_ ID2D1Factory1* direct2DFactory);
    HRESULT createDeviceSwapChainBitmap();

    void resizeSwapChainBitmap(_In_ const LONG newWidth, _In_ const LONG newHeight)
    {
        if (m_direct2DContext && newWidth && newHeight)
        {
            m_direct2DContext->SetTarget(nullptr);
            if (FAILED(m_swapChain->ResizeBuffers(0, newWidth, newHeight, DXGI_FORMAT_UNKNOWN, 0)) ||
                FAILED(createDeviceSwapChainBitmap()))
            {
                releaseDevice();
            }
        }
    }

    static ATLADD com_ptr<ID3D11Device> m_direct3DDevice;
    static ATLADD com_ptr<ID2D1Device> m_direct2DDevice;
    ATLADD com_ptr<ID2D1DeviceContext> m_direct2DContext;
    ATLADD com_ptr<IDXGISwapChain1> m_swapChain;
};
#pragma endregion directx_render declaration

#pragma region window_impl
template <typename T>
class window_impl abstract:
    public ATL::CWindowImpl<window_impl<T>>,
    protected wi_factories,
    protected directx_render
{
public:
    static ATL::CWndClassInfo& GetWndClassInfo();

    static LPCTSTR getSuperclassedWndClassName()
    {
        return nullptr;
    }

    _Check_return_ virtual HWND create(_In_ const HWND parent) = 0;

    virtual BOOL ProcessWindowMessage(
        _In_ HWND hwnd,
        _In_ UINT message,
        _In_ WPARAM wParam,
        _In_ LPARAM lParam,
        _Inout_ LRESULT& lResult,
        _In_ DWORD msgMapID) override;

    static const unsigned int m_cnWindowMargin = 11;
    static const unsigned int m_cnUnrelatedCtrlsSpace = 11;


protected:
    virtual LRESULT createHandler()
    {
        return SUCCEEDED(createFactories()) ? 0 : -1;
    }

    virtual void destroyHandler()
    {
    }

    virtual void sizeHandler(_In_ const LONG newWidth, _In_ const LONG newHeight)
    {
        resizeSwapChainBitmap(newWidth, newHeight);
    }

    template <typename U, typename... Args>
    std::unique_ptr<U> createChild(_In_ Args&&... args) const
    {
        auto child = std::make_unique<U>(std::forward<Args>(args)...);
        if (child)
        {
            if (!child->create(m_hWnd))
            {
                child.reset();
            }
        }
        return child;
    }

    LONG getRectWidth(_In_ const RECT& rect) const
    {
        return rect.right - rect.left;
    }

    LONG getRectHeight(_In_ const RECT& rect) const
    {
        return rect.bottom - rect.top;
    }

    HRESULT createTextLayoutForBodyTitle(
        _In_reads_z_(length) LPCTSTR text,
        _In_ const size_t length,
        _In_ const D2D1_SIZE_F* size,
        _COM_Outptr_result_maybenull_ IDWriteTextLayout** resultTextLayout) const
    {
        return directx_toolkit::createTextLayoutForBodyTitle(
            m_directWriteFactory.get(), m_hWnd, text, length, size, resultTextLayout);
    }

    HRESULT createTextLayoutForBodyText(
        _In_reads_z_(length) LPCTSTR text,
        _In_ const size_t length,
        _In_ const D2D1_SIZE_F* size,
        _COM_Outptr_result_maybenull_ IDWriteTextLayout** resultTextLayout) const
    {
        return directx_toolkit::createTextLayoutForBodyText(
            m_directWriteFactory.get(), m_hWnd, text, length, size, resultTextLayout);
    }

    HRESULT createTextFormatForBodyText(
        _COM_Outptr_result_maybenull_ IDWriteTextFormat** resultTextFormat) const
    {
        return directx_toolkit::createTextFormatForBodyText(m_directWriteFactory.get(), m_hWnd, resultTextFormat);
    }

    std::unique_ptr<WAPI animation> m_animation;


private:
    // Narrow access to render-aware objects.
    using directx_render::createDirect3DDevice;
    using directx_render::createDirect2DDevice;
    using directx_render::createDevice;
//    using directx_render::releaseDevice; -- `child_window_impl::destroyHandler()` needs an access to the method.
    using directx_render::render;
    using directx_render::createDeviceSwapChainBitmap;
    using directx_render::resizeSwapChainBitmap;

    using directx_render::m_direct3DDevice;
    using directx_render::m_direct2DDevice;
    using directx_render::m_swapChain;
};
#pragma endregion window_impl declaration

ATLADD_END
