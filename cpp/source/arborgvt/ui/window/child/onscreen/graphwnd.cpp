#include "ui\window\child\onscreen\graphwnd.h"

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
_Check_return_ HWND graph_window::create(_In_ const HWND parent, _In_ DWORD style, _In_ DWORD exStyle)
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
        bHandled = base_class_t::ProcessWindowMessage(hWnd, nMessage, nWParam, nLParam, nLResult, nMsgMapID);
    }
    return bHandled;
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
void graph_window::draw()
{
    m_direct2DContext->Clear(D2D1::ColorF {GetSysColor(COLOR_WINDOW), 1.0f});
}
ATLADD_END
