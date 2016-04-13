#include "service/functype.h"
#include "service/strgutil.h"
#include "service/winapi/theme.h"
#include "ui/window/top/twinimpl.h"
#include "ui/window/wi.h"
#include <Uxtheme.h>
#include <vsstyle.h>
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
 * >hwnd
 * The method retrieves per-monitor DPI settings for a monitor where the root (in the chain of parent windows) window of
 * 'hwnd' is located (for Windows 8.1 and above).
 * >dpiX
 * Pointer to variable that receives number of logical pixels per horizontal inch.
 * >dpiY
 * Pointer to variable that receives number of logical pixels per vertical inch.
 *
 * Returns:
 * Standard HRESULT.
 *
 * Remarks:
 * The method uses the 'GetAncestor' function to get the root window of the 'hwnd'. This helps in situations where the
 * root and a child windows locate on monitors with different DPI. But since in this situation the application changes
 * size of the root window, all child windows have to render with DPI of a monitor where the root window is located.
 */
HRESULT directx_toolkit::getDpiForMonitor(_In_ const HWND hwnd, _Out_ float* dpiX, _Out_ float* dpiY) const
{
    *dpiX = 96.0f;
    *dpiY = 96.0f;
    HRESULT hr;
    HMODULE lib = nullptr;
    get_dpi_for_monitor_func_t getDPIForMonitor =
        getFunction<get_dpi_for_monitor_func_t>(TEXT("shcore.dll"), "GetDpiForMonitor", lib);
    if (getDPIForMonitor)
    {
        HMONITOR monitor = MonitorFromWindow(GetAncestor(hwnd, GA_ROOT), MONITOR_DEFAULTTONEAREST);
        UINT integralDPIX;
        UINT integralDPIY;
        hr = (getDPIForMonitor)(monitor, MDT_EFFECTIVE_DPI, &integralDPIX, &integralDPIY);
        if (SUCCEEDED(hr))
        {
            *dpiX = static_cast<float> (integralDPIX);
            *dpiY = static_cast<float> (integralDPIY);
        }
    }
    else
    {
        HDC dc = ::GetDC(nullptr);
        *dpiX = static_cast<float> (GetDeviceCaps(dc, LOGPIXELSX));
        *dpiY = static_cast<float> (GetDeviceCaps(dc, LOGPIXELSY));
        ::ReleaseDC(nullptr, dc);
        hr = S_OK;
    }
    if (lib)
    {
        FreeLibrary(lib);
    }
    return hr;
}


/**
 * Creates DirectWrite's IDWriteTextLayout object that represents the specified text as title of a text.
 *
 * Parameters:
 * >directWriteFactory
 * Pointer to instantiated 'IDWriteFactory' object used to create result 'IDWriteTextLayout'.
 * >hwnd
 * The method uses Visual Styles API to get font for the result object. This is handle of the window for which visual
 * theme data is required.
 * >text
 * Source text.
 * >length
 * Size of the 'text'.
 * >size
 * Layout box size.
 * >resultTextLayout
 * Pointer to pointer that receives the result object (if the method succeeded in terms of the HRESULT).
 *
 * Returns:
 * Standard HRESULT.
 */
HRESULT directx_toolkit::createTextLayoutForBodyTitle(
    _In_ IDWriteFactory* directWriteFactory,
    _In_ const HWND hwnd,
    _In_reads_z_(length) LPCTSTR text,
    _In_ const size_t length,
    _In_ const D2D1_SIZE_F* size,
    _COM_Outptr_result_maybenull_ IDWriteTextLayout** resultTextLayout) const
{
    LOGFONT font {};
    WAPI theme_t theme {OpenThemeData(hwnd, VSCLASS_TEXTSTYLE)};
    HRESULT hr = theme ? GetThemeFont(theme.get(), nullptr, TEXT_BODYTITLE, 0, TMT_FONT, &font) : E_FAIL;
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
        hr = createTextLayout(directWriteFactory, text, length, &font, size, resultTextLayout);
    }
    else
    {
        *resultTextLayout = nullptr;
    }
    return hr;
}


/**
 * Creates DirectWrite's IDWriteTextLayout object that represents the specified text as body of a text.
 *
 * Parameters:
 * >directWriteFactory
 * Pointer to instantiated 'IDWriteFactory' object used to create result 'IDWriteTextLayout'.
 * >hwnd
 * The method uses Visual Styles API to get font for the result object. This is handle of the window for which visual
 * theme data is required.
 * >text
 * Source text.
 * >length
 * Size of the 'text'.
 * >size
 * Layout box size.
 * >resultTextLayout
 * Pointer to pointer that receives the result object (if the method succeeded in terms of the HRESULT).
 *
 * Returns:
 * Standard HRESULT.
 */
HRESULT directx_toolkit::createTextLayoutForBodyText(
    _In_ IDWriteFactory* directWriteFactory,
    _In_ const HWND hwnd,
    _In_reads_z_(length) LPCTSTR text,
    _In_ const size_t length,
    _In_ const D2D1_SIZE_F* size,
    _COM_Outptr_result_maybenull_ IDWriteTextLayout** resultTextLayout) const
{
    LOGFONT font {};
    WAPI theme_t theme {OpenThemeData(hwnd, VSCLASS_TEXTSTYLE)};
    HRESULT hr = theme ? GetThemeFont(theme.get(), nullptr, TEXT_BODYTEXT, 0, TMT_FONT, &font) : E_FAIL;
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
            font = nonClientMetrics.lfMessageFont;
            hr = S_OK;
        }
    }
    if (SUCCEEDED(hr))
    {
        hr = createTextLayout(directWriteFactory, text, length, &font, size, resultTextLayout);
    }
    else
    {
        *resultTextLayout = nullptr;
    }
    return hr;
}


/**
 * Loads image from the specified resource and creates Direct2D bitmap object for that image.
 *
 * Parameters:
 * >resourceName
 * Image resource name.
 * >resourceType
 * Image resource type.
 * >imagingFactory
 * Pointer to an instance of IWICImagingFactory object.
 * >bitmapSource
 * Receives loaded bitmap.
 *
 * Returns:
 * Standard HRESULT code.
 */
HRESULT directx_toolkit::loadBitmapFromResource(
    _In_z_ LPCTSTR resourceName,
    _In_z_ LPCTSTR resourceType,
    _In_ IWICImagingFactory* imagingFactory,
    _COM_Outptr_result_maybenull_ IWICFormatConverter** bitmapSource) const
{
    ATLADD com_ptr<IWICFormatConverter> converter {};
    HRESULT hr = E_FAIL;
    HRSRC resource = FindResource(ATL::_AtlBaseModule.GetResourceInstance(), resourceName, resourceType);
    if (nullptr != resource)
    {
        HGLOBAL data = LoadResource(ATL::_AtlBaseModule.GetResourceInstance(), resource);
        if (nullptr != data)
        {
            void* resourceData = LockResource(data);
            DWORD length = SizeofResource(ATL::_AtlBaseModule.GetResourceInstance(), resource);
            if (resourceData && length)
            {
                // Create a WIC stream to map onto the memory.
                ATLADD com_ptr<IWICStream> stream {};
                hr = imagingFactory->CreateStream(stream.getAddressOf());
                if (SUCCEEDED(hr))
                {
                    hr = stream->InitializeFromMemory(reinterpret_cast<BYTE*> (resourceData), length);
                    if (SUCCEEDED(hr))
                    {
                        // Prepare decoder for the stream, the first image frame and conveter (to change image color
                        // format).
                        ATLADD com_ptr<IWICBitmapDecoder> decoder {};
                        hr = imagingFactory->CreateDecoderFromStream(
                            stream.get(), nullptr, WICDecodeMetadataCacheOnLoad, decoder.getAddressOf());
                        if (SUCCEEDED(hr))
                        {
                            ATLADD com_ptr<IWICBitmapFrameDecode> frame {};
                            hr = decoder->GetFrame(0, frame.getAddressOf());
                            if (SUCCEEDED(hr))
                            {
                                hr = imagingFactory->CreateFormatConverter(converter.getAddressOf());
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
    *bitmapSource = SUCCEEDED(hr) ? converter.detach() : nullptr;
    return hr;
}


/**
 * Loads image from the specified resource and creates Direct2D bitmap object for that image.
 *
 * Parameters:
 * >resourceName
 * Image resource name.
 * >resourceType
 * Image resource type.
 * >imagingFactory
 * Pointer to an instance of IWICImagingFactory object.
 * >dc
 * Direct2D context.
 * >resultBitmap
 * Receives loaded bitmap.
 *
 * Returns:
 * Standard HRESULT code.
 *
 * Remarks:
 * Since 'ID2D1DeviceContext::CreateBitmapFromWicBitmap' can't be called simultaneously from multiple threads, caller
 * should synchronize access to the 'dc'.
 */
HRESULT directx_toolkit::loadBitmapFromResource(
    _In_z_ LPCTSTR resourceName,
    _In_z_ LPCTSTR resourceType,
    _In_ IWICImagingFactory* imagingFactory,
    _In_ ID2D1DeviceContext* dc,
    _COM_Outptr_result_maybenull_ ID2D1Bitmap1** resultBitmap) const
{
    ATLADD com_ptr<ID2D1Bitmap1> bitmap {};
    ATLADD com_ptr<IWICFormatConverter> converter {};
    HRESULT hr = loadBitmapFromResource(resourceName, resourceType, imagingFactory, converter.getAddressOf());
    if (SUCCEEDED(hr))
    {
        hr = dc->CreateBitmapFromWicBitmap(converter.get(), bitmap.getAddressOf());
    }
    *resultBitmap = SUCCEEDED(hr) ? bitmap.detach() : nullptr;
    return hr;
}


/**
 * Loads image from the specified URI (local file system only) and creates Direct2D bitmap object for that image.
 *
 * Parameters:
 * >uri
 * Image URI.
 * >imagingFactory
 * Pointer to an instance of IWICImagingFactory object.
 * >dc
 * Direct2D context.
 * >resultBitmap
 * Receives loaded bitmap.
 *
 * Returns:
 * New bitmap object.
 *
 * Remarks:
 * Since 'ID2D1DeviceContext::CreateBitmapFromWicBitmap' can't be called simultaneously from multiple threads, caller
 * should synchronize access to the 'dc'.
 *
 * Until the result bitmap ain't released the source 'uri' file is locked. WIC's method (
 * 'IWICImagingFactory::CreateDecoderFromFilename') calls the 'CreateFile' with only FILE_SHARE_READ shared mode.
 */
HRESULT directx_toolkit::loadBitmapFromFile(
    _In_z_ LPCTSTR uri,
    _In_ IWICImagingFactory* imagingFactory,
    _In_ ID2D1DeviceContext* dc,
    _COM_Outptr_result_maybenull_ ID2D1Bitmap1** resultBitmap) const
{
    ATLADD com_ptr<ID2D1Bitmap1> bitmap {};
    // Prepare decoder for the file, the first image frame and conveter (to change image color format).
    ATLADD com_ptr<IWICBitmapDecoder> decoder {};
    HRESULT hr = imagingFactory->CreateDecoderFromFilename(
        uri, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, decoder.getAddressOf());
    if (SUCCEEDED(hr))
    {
        ATLADD com_ptr<IWICBitmapFrameDecode> frame {};
        hr = decoder->GetFrame(0, frame.getAddressOf());
        if (SUCCEEDED(hr))
        {
            ATLADD com_ptr<IWICFormatConverter> converter {};
            hr = imagingFactory->CreateFormatConverter(converter.getAddressOf());
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
                    hr = dc->CreateBitmapFromWicBitmap(converter.get(), bitmap.getAddressOf());
                }
            }
        }
    }
    *resultBitmap = SUCCEEDED(hr) ? bitmap.detach() : nullptr;
    return hr;
}


/**
 * Creates DirectWrite's IDWriteTextLayout object that represents the specified text.
 *
 * Parameters:
 * >directWriteFactory
 * Pointer to instantiated 'IDWriteFactory' object used to create result 'IDWriteTextLayout'.
 * >text
 * Source text.
 * >length
 * Size of the 'text'.
 * >logFont
 * Pointer to initialized LOGFONT structure for creating text format.
 * >size
 * Layout box size.
 * >resultTextLayout
 * Pointer to pointer that receives the result object (if the method succeeded in terms of the HRESULT).
 *
 * Returns:
 * Standard HRESULT.
 */
HRESULT directx_toolkit::createTextLayout(
    _In_ IDWriteFactory* directWriteFactory,
    _In_reads_z_(length) LPCTSTR text,
    _In_ const size_t length,
    _In_ const LOGFONT* logFont,
    _In_ const D2D1_SIZE_F* size,
    _COM_Outptr_result_maybenull_ IDWriteTextLayout** resultTextLayout) const
{
    ATLADD com_ptr<IDWriteTextLayout> textLayout {};
    HRESULT hr = E_POINTER;
    if (directWriteFactory && text)
    {
        string_util::const_ptr_t stringUtil = string_util::getInstance();
        STLADD string_unique_ptr_t locale = stringUtil->getCurrentUserLocale();
        ATLADD com_ptr<IDWriteTextFormat> textFormat;
        hr = directWriteFactory->CreateTextFormat(
            logFont->lfFaceName,
            nullptr,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            abs(logFont->lfHeight) * 72.0f / 96.0f,
            locale ? locale->c_str() : L"",
            textFormat.getAddressOf());
        if (SUCCEEDED(hr))
        {
            hr = directWriteFactory->CreateTextLayout(
                text,
                static_cast<UINT32> (length),
                textFormat.get(),
                size->width,
                size->height,
                textLayout.getAddressOf());
            if (SUCCEEDED(hr))
            {
                ATLADD com_ptr<IDWriteTypography> typography {};
                hr = directWriteFactory->CreateTypography(typography.getAddressOf());
                if (SUCCEEDED(hr))
                {
                    DWRITE_FONT_FEATURE fontFeature;
                    fontFeature.nameTag = DWRITE_FONT_FEATURE_TAG_STYLISTIC_SET_7;
                    fontFeature.parameter = 1;
                    typography->AddFontFeature(fontFeature);
                    hr = textLayout->SetTypography(
                        typography.get(), DWRITE_TEXT_RANGE {0, static_cast<UINT32> (length)});
                }
            }
        }
    }
    *resultTextLayout = SUCCEEDED(hr) ? textLayout.detach() : nullptr;
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
 * >direct2DFactory
 * Pointer to initialized Direct2D resources factory.
 *
 * Returns:
 * Standard HRESULT.
 *
 * Remarks:
 * As its counterpart 'directx_render::createDirect3DDevice', this method stores result in the static data member.
 */
HRESULT directx_render::createDirect2DDevice(_In_ ID2D1Factory1* direct2DFactory)
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
                hr = direct2DFactory->CreateDevice(dxgiDevice.get(), m_direct2DDevice.getAddressOf());
            }
        }
    }
    return hr;
}


/**
 * Creates Direct3D and Direct2D devices and resources to use when render the specified window.
 *
 * Parameters:
 * hwnd
 * Handle to the target window.
 * >direct2DFactory
 * Pointer to initialized Direct2D resources factory. Used to create Direct2D device with
 * 'directx_render::createDirect2DDevice' method.
 *
 * Returns:
 * Standard HRESULT.
 */
HRESULT directx_render::createDevice(_In_ const HWND hwnd, _In_ ID2D1Factory1* direct2DFactory)
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
                        m_direct3DDevice.get(), hwnd, &swapChainDesc, nullptr, nullptr, m_swapChain.getAddressOf());
                    if (SUCCEEDED(hr))
                    {
                        hr = dxgiDevice->SetMaximumFrameLatency(5);
                        if (SUCCEEDED(hr))
                        {
                            hr = createDirect2DDevice(direct2DFactory);
                            if (SUCCEEDED(hr))
                            {
                                hr = m_direct2DDevice->CreateDeviceContext(
                                    D2D1_DEVICE_CONTEXT_OPTIONS_NONE, m_direct2DContext.getAddressOf());
                                if (SUCCEEDED(hr))
                                {
                                    float dpiX;
                                    float dpiY;
                                    if (FAILED(getDpiForMonitor(hwnd, &dpiX, &dpiY)))
                                    {
                                        direct2DFactory->GetDesktopDpi(&dpiX, &dpiY);
                                    }
                                    m_direct2DContext->SetDpi(dpiX, dpiY);
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
 * >hwnd
 * Handle to the target window.
 * >direct2DFactory
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
void directx_render::render(_In_ const HWND hwnd, _In_ ID2D1Factory1* direct2DFactory)
{
    HRESULT hr = S_OK;
    if (!isDirect2DContextCreated())
    {
        hr = createDevice(hwnd, direct2DFactory);
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

        // I'm accessing D3D resources directly without Direct2D's knowledge, so I must acquire and apply the Direct2D
        // factory lock.
        {
            DXU direct2d_factory_lock lock {direct2DFactory};
            DXGI_PRESENT_PARAMETERS presentParameters = {};
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
template class window_impl<top_window_impl>;
#pragma endregion window_impl template classes instantiation

#pragma region window_impl
/**
 * Gets information about "window class". Called by ATL's CWindowImplBaseT<T, U> class (by its Create method).
 *
 * Parameters:
 * None.
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
 * >hwnd
 * Handle of this window.
 * >message
 * Message to process.
 * >wParam
 * Additional message information. The contents of this parameter depend on the value of the message parameter.
 * >lParam
 * Additional message information. The contents of this parameter depend on the value of the message parameter.
 * >lResult
 * The result of the message processing and depends on the message sent (message).
 * >msgMapID
 * The identifier of the message map that will process the message.
  *
 * Returns:
 * Non-zero value if the message was processed, or zero otherwise.
*/
template <typename T>
BOOL window_impl<T>::ProcessWindowMessage(
    _In_ HWND hwnd,
    _In_ UINT message,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    _Inout_ LRESULT& lResult,
    _In_ DWORD msgMapID)
{
    UNREFERENCED_PARAMETER(msgMapID);

    BOOL handled;
    switch (message)
    {
        case WM_CREATE:
        {
            lResult = createHandler();
            handled = FALSE;
        }
        break;

        case WM_DESTROY:
        {
            destroyHandler();
            lResult = 0;
            handled = FALSE;
        }
        break;

        case WM_WINDOWPOSCHANGED:
        {
            auto windowPos = reinterpret_cast<WINDOWPOS*> (lParam);
            if (WS_CHILD & GetWindowLongPtr(GWL_STYLE))
            {
                // Child windows in this application don't have non-client area; use window size.
                if (!((SWP_NOSIZE | SWP_HIDEWINDOW) & windowPos->flags))
                {
                    sizeHandler(windowPos->cx, windowPos->cy);
                }
            }
            else if ((!(SWP_NOSIZE & windowPos->flags)) || (SWP_SHOWWINDOW & windowPos->flags))
            {
                RECT rect;
                GetClientRect(&rect);
                sizeHandler(rect.right, rect.bottom);
            }
            lResult = 0;
            handled = TRUE;
        }
        break;

        case WM_PAINT:
        {
            if (::GetUpdateRect(hwnd, nullptr, FALSE))
            {
                PAINTSTRUCT ps;
                HDC dc = ::BeginPaint(hwnd, &ps);
                if (dc)
                {
                    render(hwnd, m_direct2DFactory.get());
                }
                ::EndPaint(hwnd, &ps);
            }
            lResult = 0;
            handled = TRUE;
        }
        break;

        case WM_DPICHANGED:
        {
            if (m_direct2DContext)
            {
                m_direct2DContext->SetDpi(LOWORD(wParam), HIWORD(wParam));
            }
            lResult = 0;
            handled = TRUE;
        }
        break;

        default:
        {
            handled = FALSE;
        }
    }

    return handled;
}
#pragma endregion window_impl template

ATLADD_END
