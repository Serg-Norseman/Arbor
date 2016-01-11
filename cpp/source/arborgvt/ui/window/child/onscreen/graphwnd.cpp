#include "ui\window\child\onscreen\graphwnd.h"
#include <random>

ATLADD_BEGIN
/**
 * Creates this window.
 *
 * Parameters:
 * >hParent
 * Handle to the parent window.
 *
 * Returns:
 * Handle to this window.
 */
_Check_return_ HWND graph_window::create(_In_ const HWND hParent)
{
    typedef ATL::CWinTraits<WS_CHILD | WS_CLIPSIBLINGS | WS_TABSTOP | WS_VISIBLE, WS_EX_NOPARENTNOTIFY> style_traits_t;
    return create(hParent, style_traits_t::GetWndStyle(0), style_traits_t::GetWndExStyle(0));
}


/**
 * Creates this window.
 *
 * Parameters:
 * >parent
 * Handle to the parent window.
 * >style
 * The style of the window being created.
 * >exStyle
 * The extended window style of the window being created.
 *
 * Returns:
 * Handle to this window.
 */
_Check_return_ HWND graph_window::create(_In_opt_ const HWND parent, _In_ DWORD style, _In_ DWORD exStyle)
{
    return Create(parent, ATL::_U_RECT {nullptr}, nullptr, style, exStyle, 0U, nullptr);
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
BOOL graph_window::ProcessWindowMessage(
    _In_ HWND hWnd,
    _In_ UINT nMessage,
    _In_ WPARAM nWParam,
    _In_ LPARAM nLParam,
    _Inout_ LRESULT& nLResult,
    _In_ DWORD nMsgMapID)
{
    BOOL bHandled;
    if (m_dpiChangedMessage == nMessage)
    {
        base_class_t::ProcessWindowMessage(hWnd, WM_DPICHANGED, nWParam, nLParam, nLResult, nMsgMapID);
        bHandled = TRUE;
    }
    else
    {
        switch (nMessage)
        {
            case WM_HSCROLL:
            {
                scrollHandler(SB_HORZ, LOWORD(nWParam), HIWORD(nWParam));
                nLResult = 0;
                bHandled = TRUE;
            }
            break;

            case WM_VSCROLL:
            {
                scrollHandler(SB_VERT, LOWORD(nWParam), HIWORD(nWParam));
                nLResult = 0;
                bHandled = TRUE;
            }
            break;

            case WM_LBUTTONUP:
            {
                float fDPIX;
                float fDPIY;
                if (m_direct2DContext && SUCCEEDED(getDpiForMonitor(m_hWnd, &fDPIX, &fDPIY)))
                {
                    D2D1_MATRIX_3X2_F transform;
                    m_direct2DContext->GetTransform(&transform);
                    D2D1_SIZE_F offset {};
                    BOOL contains;
                    D2D1_POINT_2F point = D2D1::Point2F(
                        physicalToLogical(GET_X_LPARAM(nLParam), fDPIX) - transform._31,
                        physicalToLogical(GET_Y_LPARAM(nLParam), fDPIY) - transform._32);
                    if (SUCCEEDED(m_ellipse->FillContainsPoint(point, nullptr, &contains)) && contains)
                    {
                        std::random_device rd {};
                        std::mt19937 gen {rd()};
                        std::uniform_real_distribution<float> dist {0.0f, 1.0f};
                        m_color = D2D1::ColorF {dist(gen), dist(gen), dist(gen), 1.0f};
                        Invalidate(FALSE);
                        UpdateWindow();
                    }
                }
                nLResult = 0;
                bHandled = TRUE;
            }
            break;

            default:
            {
                bHandled = base_class_t::ProcessWindowMessage(hWnd, nMessage, nWParam, nLParam, nLResult, nMsgMapID);
            }
        }
    }

    return bHandled;
}


/**
 * Handles WM_CREATE message.
 *
 * Parameters:
 * None.
 *
 * Returns:
 * N/A.
 */
LRESULT graph_window::createHandler()
{
    LRESULT nResult = base_class_t::createHandler();
    if (!nResult)
    {
        RECT rect;
        GetClientRect(&rect);
        ATLADD com_ptr<ID2D1GeometrySink> s {};
        m_direct2DFactory->CreatePathGeometry(m_g.getAddressOf());
        m_g->Open(s.getAddressOf());
        const float cfStep = 30.0f;
        for (unsigned int nIt = 1; static_cast<unsigned int> (rect.right / cfStep) >= nIt; ++nIt)
        {
            s->BeginFigure(D2D1::Point2F(std::round(cfStep * nIt) + 0.5f, 0.0f), D2D1_FIGURE_BEGIN_HOLLOW);
            s->AddLine(D2D1::Point2F(std::round(cfStep * nIt) + 0.5f, static_cast<float> (rect.bottom)));
            s->EndFigure(D2D1_FIGURE_END_OPEN);
        }
        for (unsigned int nIt = 1; static_cast<unsigned int> (rect.bottom / cfStep) >= nIt; ++nIt)
        {
            s->BeginFigure(D2D1::Point2F(0.0f, std::round(cfStep * nIt) + 0.5f), D2D1_FIGURE_BEGIN_HOLLOW);
            s->AddLine(D2D1::Point2F(static_cast<float> (rect.right), std::round(cfStep * nIt) + 0.5f));
            s->EndFigure(D2D1_FIGURE_END_OPEN);
        }
        s->Close();
        m_direct2DFactory->CreateEllipseGeometry(D2D1::Ellipse(D2D1::Point2F(
            rect.right * 0.5f, rect.bottom * 0.5f), rect.right * 0.5f, rect.bottom * 0.5f), m_ellipse.getAddressOf());
        m_directWriteFactory->CreateTextFormat(
            L"Segoe UI",
            nullptr,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            10.0f,
            L"ru-ru",
            m_tf.getAddressOf());
    }
    return nResult;
}


/**
 * WM_SIZE message handler.
 *
 * Parameters:
 * >nResizingType
 * The type of resizing requested.
 * >nNewWidth
 * The new width of the window.
 * >nNewHeight
 * The new height of the window.
 *
 * Returns:
 * N/A.
 */
void graph_window::sizeHandler(_In_ const LONG nNewWidth, _In_ const LONG nNewHeight)
{
    base_class_t::sizeHandler(nNewWidth, nNewHeight);
    float fDPIX;
    float fDPIY;
    if (SUCCEEDED(getDpiForMonitor(m_hWnd, &fDPIX, &fDPIY)))
    {
        RECT rect;
        GetClientRect(&rect);
        ATLADD com_ptr<ID2D1GeometrySink> s {};
        m_g.reset();
        m_direct2DFactory->CreatePathGeometry(m_g.getAddressOf());
        m_g->Open(s.getAddressOf());
        const float cfStep = 30.0f;
        for (unsigned int nIt = 1; static_cast<unsigned int> (rect.right / cfStep) >= nIt; ++nIt)
        {
            s->BeginFigure(D2D1::Point2F(std::round(cfStep * nIt) + 0.5f, 0.0f), D2D1_FIGURE_BEGIN_HOLLOW);
            s->AddLine(D2D1::Point2F(std::round(cfStep * nIt) + 0.5f, static_cast<float> (rect.bottom)));
            s->EndFigure(D2D1_FIGURE_END_OPEN);
        }
        for (unsigned int nIt = 1; static_cast<unsigned int> (rect.bottom / cfStep) >= nIt; ++nIt)
        {
            s->BeginFigure(D2D1::Point2F(0.0f, std::round(cfStep * nIt) + 0.5f), D2D1_FIGURE_BEGIN_HOLLOW);
            s->AddLine(D2D1::Point2F(static_cast<float> (rect.right), std::round(cfStep * nIt) + 0.5f));
            s->EndFigure(D2D1_FIGURE_END_OPEN);
        }
        s->Close();
        m_ellipse.reset();
        m_direct2DFactory->CreateEllipseGeometry(
            D2D1::Ellipse(D2D1::Point2F(physicalToLogical(rect.right, fDPIX) * 0.5f, 120.0f), 60.0f, 60.0),
            m_ellipse.getAddressOf());
    }
}


/**
 * Renders this window.
 *
 * Parameters:
 * None.
 *
 * Returns:
 * N/A.
 */
void graph_window::draw() const
{
    m_direct2DContext->Clear(D2D1::ColorF {GetSysColor(COLOR_WINDOW), 1.0f});
    const float cfStep = 30.0f;
    RECT rect;
    GetClientRect(&rect);
    m_brush->SetOpacity(0.25f);
    m_direct2DContext->DrawGeometry(m_g.get(), m_brush.get());
    m_brush->SetOpacity(1.0f);
    TCHAR sz[8];
    for (unsigned int nIt = 1; static_cast<unsigned int> (rect.right / cfStep) >= nIt; nIt += 2)
    {
        int nLength = _stprintf_s(sz, L"%.1f", std::round(cfStep * nIt) + 0.5f);
        m_direct2DContext->DrawText(
            sz,
            nLength,
            m_tf.get(),
            D2D1::RectF(cfStep * nIt, 0.0f, cfStep * (nIt + 2), static_cast<float> (rect.bottom)),
            m_brush.get());
    }
    for (unsigned int nIt = 1; static_cast<unsigned int> (rect.bottom / cfStep) >= nIt; nIt += 2)
    {
        int nLength = _stprintf_s(sz, L"%.1f", std::round(cfStep * nIt) + 0.5f);
        m_direct2DContext->DrawText(
            sz,
            nLength,
            m_tf.get(),
            D2D1::RectF(0.0f, cfStep * nIt, cfStep * 3.0f, cfStep * (nIt + 1)),
            m_brush.get());
    }
    m_ellipseBrush->SetColor(m_color);
    m_direct2DContext->FillGeometry(m_ellipse.get(), m_ellipseBrush.get());
    // C++ overloaded function template `wcscpy_s` doesn't have `_Out_writes_z_` SAL annotation. Therefore to shut the
    // `C6054' warning down I have to make `sz` zero-terminated explicitly.
    _tcscpy_s(sz, L"\u2190click!");
    // This is a sample code so I can accept such code here.
    sz[_countof(L"\u2190click!") - 1] = L'\x00';
    size_t nLength = _tcscnlen(sz, _countof(sz));
    // Shut the `C6385' warning down with the following, very silly code. It's definitely prefast noise. This and above
    // "fixes" made for MSVC 2015 Update 1.
    if (0 < nLength)
    {
        D2D1_ELLIPSE ellipse;
        m_ellipse->GetEllipse(&ellipse);
        D2D1_POINT_2F origin = D2D1::Point2F(
            ellipse.point.x + ellipse.radiusX * cos(atan(1.0f)),
            ellipse.point.y + ellipse.radiusY * cos(atan(1.0f)));
        D2D1_SIZE_F size = m_direct2DContext->GetSize();
        m_direct2DContext->DrawText(
            sz,
            static_cast<UINT32> (nLength),
            m_tf.get(),
            D2D1::RectF(origin.x, origin.y, size.width, size.height),
            m_brush.get());
    }
}


/**
 * WM_HSCROLL/WM_VSCROLL messages handler.
 *
 * Parameters:
 * >nBar
 * Specifies the type of scroll bar (SB_HORZ or SB_VERT).
 * >nScrollingRequest
 * User's scrolling request.
 * >nPosition
 * The current position of the scroll box if the 'nScrollingRequest' is SB_THUMBPOSITION or SB_THUMBTRACK.
 *
 * Returns:
 * N/A.
 */
void graph_window::scrollHandler(_In_ int nBar, _In_ const WORD nScrollingRequest, _In_ const WORD nPosition)
{
    SCROLLINFO si;
    si.cbSize = sizeof(si);
    si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
    ::GetScrollInfo(::GetParent(m_hWnd), nBar, &si);
    int nPrevPos = si.nPos;
    switch (nScrollingRequest)
    {
        case SB_TOP:
        {
            si.nPos = si.nMin;
        }
        break;

        case SB_BOTTOM:
        {
            si.nPos = si.nMax;
        }
        break;

        case SB_LINEUP:
        {
            // 1 is _logical_ pixel here...
            si.nPos -= 1;
        }
        break;

        case SB_LINEDOWN:
        {
            // ...and here
            si.nPos += 1;
        }
        break;

        case SB_PAGEUP:
        {
            si.nPos -= si.nPage;
        }
        break;

        case SB_PAGEDOWN:
        {
            si.nPos += si.nPage;
        }
        break;

        case SB_THUMBTRACK:
        {
            si.nPos = nPosition;
        }
        break;

        default:
        {
        }
        break;
    }

    // Set the position and then retrieve it. Due to adjustments by Windows it may not be the same as the value
    // set.
    si.fMask = SIF_POS;
    ::SetScrollInfo(::GetParent(m_hWnd), nBar, &si, TRUE);
    ::GetScrollInfo(::GetParent(m_hWnd), nBar, &si);

    // If the position has changed, scroll the window.
    if (nPrevPos != si.nPos)
    {
        scrollContent(nBar, si.nPos);
    }
}


/**
 * Scrolls this window content.
 *
 * Parameters:
 * >nBar
 * Specifies the type of scroll bar (SB_HORZ or SB_VERT).
 * >nPos
 * Scrolling position (physical value).
 *
 * Returns:
 * N/A.
 */
void graph_window::scrollContent(_In_ int nBar, _In_ const int nPos)
{
    if (m_direct2DContext)
    {
        float fDPIX;
        float fDPIY;
        if (SUCCEEDED(getDpiForMonitor(m_hWnd, &fDPIX, &fDPIY)))
        {
            D2D1_MATRIX_3X2_F transform;
            m_direct2DContext->GetTransform(&transform);
            D2D1_SIZE_F offset {};
            if (SB_HORZ == nBar)
            {
                offset = D2D1::SizeF(physicalToLogical(-nPos, fDPIX), transform._32);
            }
            else if (SB_VERT == nBar)
            {
                offset = D2D1::SizeF(transform._31, physicalToLogical(-nPos, fDPIY));
            }
            m_direct2DContext->SetTransform(D2D1::Matrix3x2F::Translation(offset));
            Invalidate(FALSE);
            UpdateWindow();
        }
    }
}
ATLADD_END
