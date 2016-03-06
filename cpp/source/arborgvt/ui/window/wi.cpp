#include "service\functype.h"
#include "service\strgutil.h"
#include "service\winapi\theme.h"
#include "ui\window\child\onscreen\graphwnd.h"
#include "ui\window\wi.h"
#include <vssym32.h>

ATLADD_BEGIN

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "uxtheme.lib")

#pragma region wi_factories
ATLADD com_ptr<ID2D1Factory1> wi_factories::m_direct2DFactory;
ATLADD com_ptr<IDWriteFactory1> wi_factories::m_directWriteFactory;
ATLADD com_ptr<IWICImagingFactory> wi_factories::m_imagingFactory;
#pragma endregion wi_factories definition

#pragma region directx_toolkit
/**
 * Determines per-monitor DPI settings or system wide DPI settings (for systems before Windows 8.1).
 *
 * Parameters:
 * >hWnd
 * The method retrieves per-monitor DPI settings for a monitor where the root (in the chain of parent windows) window of
 * 'hWnd' is located (for Windows 8.1 and above).
 * >pfDPIX
 * Pointer to variable that receives number of logical pixels per horizontal inch.
 * >pfDPIY
 * Pointer to variable that receives number of logical pixels per vertical inch.
 *
 * Returns:
 * Standard HRESULT.
 *
 * Remarks:
 * The method uses the 'GetAncestor' function to get the root window of the 'hWnd'. This helps in situations where the
 * root and a child windows locate on monitors with different DPI. But since in this situation the application changes
 * size of the root window, all child windows have to render with DPI of a monitor where the root window is located.
 */
HRESULT directx_toolkit::getDpiForMonitor(_In_ const HWND hWnd, _Out_ float* pfDPIX, _Out_ float* pfDPIY) const
{
    *pfDPIX = 96.0f;
    *pfDPIY = 96.0f;
    HRESULT hResult;
    HMODULE hLib = nullptr;
    get_dpi_for_monitor_func_t pGetDPIForMonitor =
        getFunction<get_dpi_for_monitor_func_t>(TEXT("shcore.dll"), "GetDpiForMonitor", hLib);
    if (pGetDPIForMonitor)
    {
        HMONITOR hMonitor = MonitorFromWindow(GetAncestor(hWnd, GA_ROOT), MONITOR_DEFAULTTONEAREST);
        UINT nDPIX;
        UINT nDPIY;
        hResult = (pGetDPIForMonitor)(hMonitor, MDT_EFFECTIVE_DPI, &nDPIX, &nDPIY);
        if (SUCCEEDED(hResult))
        {
            *pfDPIX = static_cast<float> (nDPIX);
            *pfDPIY = static_cast<float> (nDPIY);
        }
    }
    else
    {
        HDC hDC = ::GetDC(nullptr);
        *pfDPIX = static_cast<float> (GetDeviceCaps(hDC, LOGPIXELSX));
        *pfDPIY = static_cast<float> (GetDeviceCaps(hDC, LOGPIXELSY));
        ::ReleaseDC(nullptr, hDC);
        hResult = S_OK;
    }
    if (hLib)
    {
        FreeLibrary(hLib);
    }
    return hResult;
}


/**
 * Creates DirectWrite's IDWriteTextLayout object that represents the specified text as title of a text.
 *
 * Parameters:
 * >pDWriteFactory
 * Pointer to instantiated 'IDWriteFactory' object used to create result 'IDWriteTextLayout'.
 * >hWnd
 * The method uses Visual Styles API to get font for the result object. This is handle of the window for which visual
 * theme data is required.
 * >pszText
 * Source text.
 * >nSize
 * Size of the 'pszText'.
 * >pSize
 * Layout box size.
 * >ppTextLayout
 * Pointer to pointer that receives the result object (if the method succeeded in terms of the HRESULT).
 *
 * Returns:
 * Standard HRESULT code.
 */
HRESULT directx_toolkit::createTextLayoutForBodyTitle(
    _In_ IDWriteFactory* pDWriteFactory,
    _In_ const HWND hWnd,
    _In_reads_z_(nSize) LPCTSTR pszText,
    _In_ const size_t nSize,
    _In_ const D2D1_SIZE_F* pSize,
    _COM_Outptr_result_maybenull_ IDWriteTextLayout** ppTextLayout) const
{
    ATLADD com_ptr<IDWriteTextFormat> textFormat {};
    HRESULT hr = createTextFormatForBodyTitle(pDWriteFactory, hWnd, textFormat.getAddressOf());
    if (SUCCEEDED(hr))
    {
        hr = createTextLayout(pDWriteFactory, pszText, nSize, textFormat.get(), pSize, ppTextLayout);
    }
    else
    {
        *ppTextLayout = nullptr;
    }
    return hr;
}


/**
 * Creates DirectWrite's IDWriteTextLayout object that represents the specified text as body of a text.
 *
 * Parameters:
 * >pDWriteFactory
 * Pointer to instantiated 'IDWriteFactory' object used to create result 'IDWriteTextLayout'.
 * >hWnd
 * The method uses Visual Styles API to get font for the result object. This is handle of the window for which visual
 * theme data is required.
 * >pszText
 * Source text.
 * >nSize
 * Size of the 'pszText'.
 * >pSize
 * Layout box size.
 * >ppTextLayout
 * Pointer to pointer that receives the result object (if the method succeeded in terms of the HRESULT).
 *
 * Returns:
 * Standard HRESULT code.
 */
HRESULT directx_toolkit::createTextLayoutForBodyText(
    _In_ IDWriteFactory* pDWriteFactory,
    _In_ const HWND hWnd,
    _In_reads_z_(nSize) LPCTSTR pszText,
    _In_ const size_t nSize,
    _In_ const D2D1_SIZE_F* pSize,
    _COM_Outptr_result_maybenull_ IDWriteTextLayout** ppTextLayout) const
{
    ATLADD com_ptr<IDWriteTextFormat> textFormat {};
    HRESULT hr = createTextFormatForBodyText(pDWriteFactory, hWnd, textFormat.getAddressOf());
    if (SUCCEEDED(hr))
    {
        hr = createTextLayout(pDWriteFactory, pszText, nSize, textFormat.get(), pSize, ppTextLayout);
    }
    else
    {
        *ppTextLayout = nullptr;
    }
    return hr;
}


/**
 * Loads image from the specified resource and creates Direct2D bitmap object for that image.
 *
 * Parameters:
 * >pszResourceName
 * Image resource name.
 * >pszResourceType
 * Image resource type.
 * >pImagingFactory
 * Pointer to an instance of IWICImagingFactory object.
 * >ppBitmapSource
 * Receives loaded bitmap.
 *
 * Returns:
 * Standard HRESULT code.
 */
HRESULT directx_toolkit::loadBitmapFromResource(
    _In_z_ LPCTSTR pszResourceName,
    _In_z_ LPCTSTR pszResourceType,
    _In_ IWICImagingFactory* pImagingFactory,
    _COM_Outptr_result_maybenull_ IWICFormatConverter** ppBitmapSource) const
{
    ATLADD com_ptr<IWICFormatConverter> converter {};
    HRESULT hr = E_FAIL;
    HRSRC hResource = FindResource(ATL::_AtlBaseModule.GetResourceInstance(), pszResourceName, pszResourceType);
    if (nullptr != hResource)
    {
        HGLOBAL hData = LoadResource(ATL::_AtlBaseModule.GetResourceInstance(), hResource);
        if (nullptr != hData)
        {
            void* pData = LockResource(hData);
            DWORD nSize = SizeofResource(ATL::_AtlBaseModule.GetResourceInstance(), hResource);
            if (pData && nSize)
            {
                // Create a WIC stream to map onto the memory.
                ATLADD com_ptr<IWICStream> stream {};
                hr = pImagingFactory->CreateStream(stream.getAddressOf());
                if (SUCCEEDED(hr))
                {
                    hr = stream->InitializeFromMemory(reinterpret_cast<BYTE*> (pData), nSize);
                    if (SUCCEEDED(hr))
                    {
                        // Prepare decoder for the stream, the first image frame and conveter (to change image color
                        // format).
                        ATLADD com_ptr<IWICBitmapDecoder> decoder {};
                        hr = pImagingFactory->CreateDecoderFromStream(
                            stream.get(), nullptr, WICDecodeMetadataCacheOnLoad, decoder.getAddressOf());
                        if (SUCCEEDED(hr))
                        {
                            ATLADD com_ptr<IWICBitmapFrameDecode> frame {};
                            hr = decoder->GetFrame(0, frame.getAddressOf());
                            if (SUCCEEDED(hr))
                            {
                                hr = pImagingFactory->CreateFormatConverter(converter.getAddressOf());
                                if (SUCCEEDED(hr))
                                {
                                    hr = converter->Initialize(
                                        frame.get(),
                                        GUID_WICPixelFormat32bppPBGRA,
                                        WICBitmapDitherTypeNone,
                                        nullptr,
                                        0,
                                        WICBitmapPaletteTypeMedianCut);
                                }
                            }
                        }
                    }
                }
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    *ppBitmapSource = SUCCEEDED(hr) ? converter.detach() : nullptr;
    return hr;
}


/**
 * Loads image from the specified resource and creates Direct2D bitmap object for that image.
 *
 * Parameters:
 * >pszResourceName
 * Image resource name.
 * >pszResourceType
 * Image resource type.
 * >pImagingFactory
 * Pointer to an instance of IWICImagingFactory object.
 * >pDC
 * Direct2D context.
 * >ppBitmap
 * Receives loaded bitmap.
 *
 * Returns:
 * Standard HRESULT code.
 *
 * Remarks:
 * Since 'ID2D1DeviceContext::CreateBitmapFromWicBitmap' can't be called simultaneously from multiple threads, caller
 * should synchronize access to the 'pDC'.
 */
HRESULT directx_toolkit::loadBitmapFromResource(
    _In_z_ LPCTSTR pszResourceName,
    _In_z_ LPCTSTR pszResourceType,
    _In_ IWICImagingFactory* pImagingFactory,
    _In_ ID2D1DeviceContext* pDC,
    _COM_Outptr_result_maybenull_ ID2D1Bitmap1** ppBitmap) const
{
    ATLADD com_ptr<ID2D1Bitmap1> bitmap {};
    ATLADD com_ptr<IWICFormatConverter> converter {};
    HRESULT hr = loadBitmapFromResource(pszResourceName, pszResourceType, pImagingFactory, converter.getAddressOf());
    if (SUCCEEDED(hr))
    {
        hr = pDC->CreateBitmapFromWicBitmap(converter.get(), bitmap.getAddressOf());
    }
    *ppBitmap = SUCCEEDED(hr) ? bitmap.detach() : nullptr;
    return hr;
}


/**
 * Loads image from the specified URI (local file system only) and creates Direct2D bitmap object for that image.
 *
 * Parameters:
 * >pszURI
 * Image URI.
 * >pImagingFactory
 * Pointer to an instance of IWICImagingFactory object.
 * >pDC
 * Direct2D context.
 * >ppBitmap
 * Receives loaded bitmap.
 *
 * Returns:
 * New bitmap object.
 *
 * Remarks:
 * Since 'ID2D1DeviceContext::CreateBitmapFromWicBitmap' can't be called simultaneously from multiple threads, caller
 * should synchronize access to the 'pDC'.
 *
 * Until the result bitmap ain't released the source 'pszURI' file is locked. WIC's method (
 * 'IWICImagingFactory::CreateDecoderFromFilename') calls the 'CreateFile' with only FILE_SHARE_READ shared mode.
 */
HRESULT directx_toolkit::loadBitmapFromFile(
    _In_z_ LPCTSTR pszURI,
    _In_ IWICImagingFactory* pImagingFactory,
    _In_ ID2D1DeviceContext* pDC,
    _COM_Outptr_result_maybenull_ ID2D1Bitmap1** ppBitmap) const
{
    ATLADD com_ptr<ID2D1Bitmap1> bitmap {};
    // Prepare decoder for the file, the first image frame and conveter (to change image color format).
    ATLADD com_ptr<IWICBitmapDecoder> decoder {};
    HRESULT hr = pImagingFactory->CreateDecoderFromFilename(
        pszURI, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, decoder.getAddressOf());
    if (SUCCEEDED(hr))
    {
        ATLADD com_ptr<IWICBitmapFrameDecode> frame {};
        hr = decoder->GetFrame(0, frame.getAddressOf());
        if (SUCCEEDED(hr))
        {
            ATLADD com_ptr<IWICFormatConverter> converter {};
            hr = pImagingFactory->CreateFormatConverter(converter.getAddressOf());
            if (SUCCEEDED(hr))
            {
                hr = converter->Initialize(
                    frame.get(),
                    GUID_WICPixelFormat32bppPBGRA,
                    WICBitmapDitherTypeNone,
                    nullptr,
                    0,
                    WICBitmapPaletteTypeMedianCut);
                if (SUCCEEDED(hr))
                {
                    hr = pDC->CreateBitmapFromWicBitmap(converter.get(), bitmap.getAddressOf());
                }
            }
        }
    }
    *ppBitmap = SUCCEEDED(hr) ? bitmap.detach() : nullptr;
    return hr;
}


/**
 * Creates DirectWrite's IDWriteTextLayout object that represents the specified text.
 *
 * Parameters:
 * >pDWriteFactory
 * Pointer to instantiated 'IDWriteFactory' object used to create result 'IDWriteTextLayout'.
 * >pszText
 * Source text.
 * >nSize
 * Size of the 'pszText'.
 * >pTextFormat
 * Text format for the resulting text layout.
 * >pSize
 * Layout box size.
 * >ppTextLayout
 * Pointer to pointer that receives the result object (if the method succeeded in terms of the HRESULT).
 *
 * Returns:
 * Standard HRESULT.
 */
HRESULT directx_toolkit::createTextLayout(
    _In_ IDWriteFactory* pDWriteFactory,
    _In_reads_z_(nSize) LPCTSTR pszText,
    _In_ const size_t nSize,
    _In_ IDWriteTextFormat* pTextFormat,
    _In_ const D2D1_SIZE_F* pSize,
    _COM_Outptr_result_maybenull_ IDWriteTextLayout** ppTextLayout) const
{
    ATLADD com_ptr<IDWriteTextLayout> textLayout {};
    HRESULT hr = E_POINTER;
    if (pszText)
    {
        hr = pDWriteFactory->CreateTextLayout(
            pszText,
            static_cast<UINT32> (nSize),
            pTextFormat,
            pSize->width,
            pSize->height,
            textLayout.getAddressOf());
        if (SUCCEEDED(hr))
        {
            ATLADD com_ptr<IDWriteTypography> typography {};
            hr = pDWriteFactory->CreateTypography(typography.getAddressOf());
            if (SUCCEEDED(hr))
            {
                DWRITE_FONT_FEATURE fontFeature;
                fontFeature.nameTag = DWRITE_FONT_FEATURE_TAG_STYLISTIC_SET_7;
                fontFeature.parameter = 1;
                typography->AddFontFeature(fontFeature);
                hr = textLayout->SetTypography(
                    typography.get(), DWRITE_TEXT_RANGE {0, static_cast<UINT32> (nSize)});
            }
        }
    }
    *ppTextLayout = SUCCEEDED(hr) ? textLayout.detach() : nullptr;
    return hr;
}


/**
 * Creates DirectWrite's IDWriteTextFormat object.
 *
 * Parameters:
 * >pDWriteFactory
 * Pointer to instantiated 'IDWriteFactory' object used to create result 'IDWriteTextFormat'.
 * >pLogFont
 * Pointer to initialized LOGFONT structure for creating text format.
 * >ppTextLayout
 * Pointer to pointer that receives the result object (if the method succeeded in terms of the HRESULT).
 *
 * Returns:
 * Standard HRESULT.
 */
HRESULT directx_toolkit::createTextFormat(
    _In_ IDWriteFactory* pDWriteFactory,
    _In_ const LOGFONT* pLogFont,
    _COM_Outptr_result_maybenull_ IDWriteTextFormat** ppTextFormat) const
{
    ATLADD com_ptr<IDWriteTextFormat> textFormat {};
    HRESULT hr = E_POINTER;
    if (pDWriteFactory)
    {
        string_util::const_ptr_t pStringUtil = string_util::getInstance();
        STLADD string_unique_ptr_t szLocale = pStringUtil->getCurrentUserLocale();
        hr = pDWriteFactory->CreateTextFormat(
            pLogFont->lfFaceName,
            nullptr,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            abs(pLogFont->lfHeight) * 72.0f / 96.0f,
            szLocale ? szLocale->c_str() : L"",
            textFormat.getAddressOf());
    }
    *ppTextFormat = SUCCEEDED(hr) ? textFormat.detach() : nullptr;
    return hr;
}


/**
* Creates DirectWrite's IDWriteTextFormat object that may be used to draw text as title of a text.
*
* Parameters:
* >pDWriteFactory
* Pointer to instantiated 'IDWriteFactory' object used to create result 'IDWriteTextFormat'.
* >hWnd
* The method uses Visual Styles API to get font for the result object. This is handle of the window for which visual
* theme data is required.
* >ppTextFormat
* Pointer to pointer that receives the result object (if the method succeeded in terms of the HRESULT).
*
* Returns:
* Standard HRESULT code.
*/
HRESULT directx_toolkit::createTextFormat(
    _In_ IDWriteFactory* pDWriteFactory,
    _In_ const HWND hWnd,
    _In_ const int nPartId,
    _COM_Outptr_result_maybenull_ IDWriteTextFormat** ppTextFormat) const
{
    LOGFONT font {};
    WAPI theme_t theme {OpenThemeData(hWnd, VSCLASS_TEXTSTYLE)};
    HRESULT hr = theme ? GetThemeFont(theme.get(), nullptr, nPartId, 0, TMT_FONT, &font) : E_FAIL;
    if (FAILED(hr))
    {
        NONCLIENTMETRICS nonClientMetrics;
        nonClientMetrics.cbSize = sizeof(NONCLIENTMETRICS);
        // I don't plan to run the application on Windows earlier than Windows 7 so I don't mess around with
        // 'NONCLIENTMETRICS::iPaddedBorderWidth' field.
        if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, nonClientMetrics.cbSize, &nonClientMetrics, 0))
        {
            // COPY a big enough structure 'LOGFONT' hoping 'GetThemeFont' will fail very rarely. At least it
            // doesn't on my Windows 10 and Windows 7 SP1.
            font = nonClientMetrics.lfCaptionFont;
            hr = S_OK;
        }
    }
    if (SUCCEEDED(hr))
    {
        hr = createTextFormat(pDWriteFactory, &font, ppTextFormat);
    }
    else
    {
        *ppTextFormat = nullptr;
    }
    return hr;
}
#pragma endregion directx_toolkit definition

#pragma region directx_render
ATLADD com_ptr<ID3D11Device> directx_render::m_direct3DDevice;
ATLADD com_ptr<ID2D1Device> directx_render::m_direct2DDevice;

/**
 * Creates Direct2D device (the 'm_direct2DDevice') linked to Direct3D device (the 'm_direct3DDevice').
 *
 * Parameters:
 * >pDirect2DFactory
 * Pointer to initialized Direct2D resources factory.
 *
 * Returns:
 * Standard HRESULT.
 *
 * Remarks:
 * As its counterpart 'directx_render::createDirect3DDevice', this method stores result in the static data member.
 */
HRESULT directx_render::createDirect2DDevice(_In_ ID2D1Factory1* pDirect2DFactory)
{
    HRESULT hr = S_OK;
    if (!m_direct2DDevice)
    {
        hr = createDirect3DDevice();
        if (SUCCEEDED(hr))
        {
            ATLADD com_ptr<IDXGIDevice> dxgiDevice {};
            hr = m_direct3DDevice->QueryInterface(dxgiDevice.getAddressOf());
            if (SUCCEEDED(hr))
            {
                hr = pDirect2DFactory->CreateDevice(dxgiDevice.get(), m_direct2DDevice.getAddressOf());
            }
        }
    }
    return hr;
}


/**
 * Creates Direct3D and Direct2D devices and resources to use when render the specified window.
 *
 * Parameters:
 * hWnd
 * Handle to the target window.
 * >pDirect2DFactory
 * Pointer to initialized Direct2D resources factory. Used to create Direct2D device with
 * 'directx_render::createDirect2DDevice' method.
 *
 * Returns:
 * Standard HRESULT.
 */
HRESULT directx_render::createDevice(_In_ const HWND hWnd, _In_ ID2D1Factory1* pDirect2DFactory)
{
    HRESULT hr = createDirect3DDevice();
    if (SUCCEEDED(hr))
    {
        ATLADD com_ptr<IDXGIDevice1> dxgiDevice {};
        hr = m_direct3DDevice->QueryInterface(dxgiDevice.getAddressOf());
        if (SUCCEEDED(hr))
        {
            ATLADD com_ptr<IDXGIAdapter> adapter {};
            hr = dxgiDevice->GetAdapter(adapter.getAddressOf());
            if (SUCCEEDED(hr))
            {
                ATLADD com_ptr<IDXGIFactory2> dxgiFactory {};
                hr = adapter->GetParent(__uuidof(dxgiFactory), reinterpret_cast<void**> (dxgiFactory.getAddressOf()));
                if (SUCCEEDED(hr))
                {
                    DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
                    swapChainDesc.Width = 0;
                    swapChainDesc.Height = 0;
                    swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
                    swapChainDesc.Stereo = FALSE;
                    swapChainDesc.SampleDesc.Count = 1;
                    swapChainDesc.SampleDesc.Quality = 0;
                    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
                    swapChainDesc.BufferCount = 2;
                    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
                    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
                    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
                    swapChainDesc.Flags = 0;
                    hr = dxgiFactory->CreateSwapChainForHwnd(
                        m_direct3DDevice.get(), hWnd, &swapChainDesc, nullptr, nullptr, m_swapChain.getAddressOf());
                    if (SUCCEEDED(hr))
                    {
                        hr = dxgiDevice->SetMaximumFrameLatency(1);
                        if (SUCCEEDED(hr))
                        {
                            hr = createDirect2DDevice(pDirect2DFactory);
                            if (SUCCEEDED(hr))
                            {
                                hr = m_direct2DDevice->CreateDeviceContext(
                                    D2D1_DEVICE_CONTEXT_OPTIONS_NONE, m_direct2DContext.getAddressOf());
                                if (SUCCEEDED(hr))
                                {
                                    float fDPIX;
                                    float fDPIY;
                                    if (FAILED(getDpiForMonitor(hWnd, &fDPIX, &fDPIY)))
                                    {
                                        pDirect2DFactory->GetDesktopDpi(&fDPIX, &fDPIY);
                                    }
                                    m_direct2DContext->SetDpi(fDPIX, fDPIY);
                                    createDeviceResources();
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return hr;
}


/**
 * Renders the specified window.
 *
 * Parameters:
 * >hWnd
 * Handle to the target window.
 * >pDirect2DFactory
 * Pointer to initialized Direct2D resources factory. Used to create (when necessary) Direct2D device with
 * 'directx_render::createDirect2DDevice' method.
 *
 * Returns:
 * N/A.
 *
 * Remarks:
 * The method never tries to initialize DirectX objects for rendering when a window doesn't own by Direct2D device
 * context ('m_bD2DContextOwner' is false). Such windows use a parent window's D2D context.
 */
void directx_render::render(_In_ const HWND hWnd, _In_ ID2D1Factory1* pDirect2DFactory)
{
    HRESULT hr = S_OK;
    if (!isDirect2DContextCreated())
    {
        hr = createDevice(hWnd, pDirect2DFactory);
        if (SUCCEEDED(hr))
        {
            hr = createDeviceSwapChainBitmap();
        }
    }
    if (SUCCEEDED(hr))
    {
        m_direct2DContext->BeginDraw();
        draw();
        m_direct2DContext->EndDraw();
        {
//            DXU direct2d_factory_lock lock {pDirect2DFactory};
            DXGI_PRESENT_PARAMETERS presentParameters = {};
            // Turn VSync on.
            hr = m_swapChain->Present1(1, 0, &presentParameters);
            if (DXGI_ERROR_DEVICE_REMOVED == hr)
            {
                releaseDevice();
            }
        }
    }
}


/**
 * Creates swap chain and assigns its the first buffer as the D2D device context target.
 *
 * Parameters:
 * None.
 *
 * Returns:
 * Standard HRESULT.
 */
HRESULT directx_render::createDeviceSwapChainBitmap()
{
    ATLADD com_ptr<IDXGISurface> surface {};
    HRESULT hr =
        m_swapChain ?
        m_swapChain->GetBuffer(0, __uuidof(IDXGISurface), reinterpret_cast<void**> (surface.getAddressOf())) :
        E_POINTER;
    if (SUCCEEDED(hr))
    {
        D2D1_BITMAP_PROPERTIES1 props = D2D1::BitmapProperties1(
            D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE));
        ATLADD com_ptr<ID2D1Bitmap1> bitmap {};
        hr = m_direct2DContext->CreateBitmapFromDxgiSurface(surface.get(), &props, bitmap.getAddressOf());
        if (SUCCEEDED(hr))
        {
            m_direct2DContext->SetTarget(bitmap.get());
        }
    }
    return hr;
}
#pragma endregion directx_render definition

#pragma region window_impl template classes
template class window_impl<graph_window>;
#pragma endregion window_impl template classes instantiation

#pragma region window_impl
/**
 * Gets information about "window class". Called by ATL's CWindowImplBaseT<T, U> class (by its Create method).
 *
 * Parameters:
 * pWndProc
 * Window procedure.
 *
 * Returns:
 * A static instance of CWndClassInfo class.
 */
template <typename T>
ATL::CWndClassInfo& window_impl<T>::GetWndClassInfo()
{
    static ATL::CWndClassInfo windowClass;
    windowClass.m_wc.cbSize = sizeof(WNDCLASSEX);
    windowClass.m_wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    windowClass.m_wc.lpfnWndProc = StartWindowProc;
    windowClass.m_wc.cbClsExtra = 0;
    windowClass.m_wc.cbWndExtra = 0;
    windowClass.m_wc.hInstance = nullptr;
    windowClass.m_wc.hIcon = nullptr;
    windowClass.m_wc.hCursor = nullptr;
    windowClass.m_wc.hbrBackground = nullptr;
    windowClass.m_wc.lpszMenuName = nullptr;
    windowClass.m_wc.lpszClassName = nullptr;
    windowClass.m_wc.hIconSm = nullptr;
    windowClass.m_lpszOrigName = T::getSuperclassedWndClassName();
    windowClass.pWndProc = nullptr;
    windowClass.m_lpszCursorID = IDC_ARROW;
    windowClass.m_bSystemCursor = TRUE;
    windowClass.m_atom = 0;
    windowClass.m_szAutoName[0] = TEXT('\x00');
    return windowClass;
}


/**
 * Handler of ATL message map.
 *
 * Parameters:
 * >hWnd
 * Handle of this window.
 * >nMessage
 * Message to process.
 * >nWParam
 * Additional message information. The contents of this parameter depend on the value of the nMessage parameter.
 * >nLParam
 * Additional message information. The contents of this parameter depend on the value of the nMessage parameter.
 * >nLResult
 * The result of the message processing and depends on the message sent (nMessage).
 * >nMsgMapID
 * The identifier of the message map that will process the message.
 *
 * Returns:
 * Non-zero value if the message was processed, or zero otherwise.
 */
template <typename T>
BOOL window_impl<T>::ProcessWindowMessage(
    _In_ HWND hWnd,
    _In_ UINT nMessage,
    _In_ WPARAM nWParam,
    _In_ LPARAM nLParam,
    _Inout_ LRESULT& nLResult,
    _In_ DWORD nMsgMapID)
{
    UNREFERENCED_PARAMETER(nMsgMapID);

    BOOL bHandled;
    switch (nMessage)
    {
        case WM_CREATE:
        {
            nLResult = createHandler();
            bHandled = FALSE;
        }
        break;

        case WM_DESTROY:
        {
            destroyHandler();
            nLResult = 0;
            bHandled = FALSE;
        }
        break;

        case WM_WINDOWPOSCHANGED:
        {
            auto pWindowPos = reinterpret_cast<WINDOWPOS*> (nLParam);
            if (WS_CHILD & GetWindowLongPtr(GWL_STYLE))
            {
                // Child windows in this application don't have non-client area; use window size.
                if (!((SWP_NOSIZE | SWP_HIDEWINDOW) & pWindowPos->flags))
                {
                    sizeHandler(pWindowPos->cx, pWindowPos->cy);
                }
            }
            else if ((!(SWP_NOSIZE & pWindowPos->flags)) || (SWP_SHOWWINDOW & pWindowPos->flags))
            {
                RECT rect;
                GetClientRect(&rect);
                sizeHandler(rect.right, rect.bottom);
            }
            nLResult = 0;
            bHandled = TRUE;
        }
        break;

        case WM_PAINT:
        {
            if (::GetUpdateRect(hWnd, nullptr, FALSE))
            {
                PAINTSTRUCT ps;
                HDC hDC = ::BeginPaint(hWnd, &ps);
                if (hDC)
                {
                    render(hWnd, m_direct2DFactory.get());
                }
                ::EndPaint(hWnd, &ps);
            }
            nLResult = 0;
            bHandled = TRUE;
        }
        break;

        case WM_DPICHANGED:
        {
            if (m_direct2DContext)
            {
                m_direct2DContext->SetDpi(LOWORD(nWParam), HIWORD(nWParam));
            }
            nLResult = 0;
            bHandled = TRUE;
        }
        break;

        default:
        {
            bHandled = FALSE;
        }
    }

    return bHandled;
}
#pragma endregion window_impl template

ATLADD_END
