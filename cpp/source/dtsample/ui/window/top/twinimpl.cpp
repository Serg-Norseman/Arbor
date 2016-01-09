#include "resource.h"
#include "ui\window\appmsg.h"
#include "ui\window\top\twinimpl.h"

#pragma comment(lib, "dwrite.lib")

ATLADD_BEGIN
#pragma region top_window_impl
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
BOOL top_window_impl::ProcessWindowMessage(
    _In_ HWND hWnd,
    _In_ UINT nMessage,
    _In_ WPARAM nWParam,
    _In_ LPARAM nLParam,
    _Inout_ LRESULT& nLResult,
    _In_ DWORD nMsgMapID)
{
    BOOL bHandled;
    switch (nMessage)
    {
        case WM_DISPLAYCHANGE:
        {
            Invalidate();
            nLResult = 0;
            bHandled = TRUE;
        }
        break;

        case WM_DPICHANGED:
        {
            EnumChildWindows(
                hWnd,
                [] (_In_ const HWND hWnd, _In_ const LPARAM data) -> BOOL
                {
                    DWORD nOwnerThreadId = ::GetWindowThreadProcessId(hWnd, nullptr);
                    if (nOwnerThreadId == GetCurrentThreadId())
                    {
                        ::SendMessage(hWnd, WM_DPICHANGED, data, 0);
                    }
                    else
                    {
                        // `WM_DPICHANGED` for some reason can't intersect threads bounds. Neither sending nor posting
                        // the message helps. I have to use an user-defined message.
                        ::SendNotifyMessage(hWnd, WM_NOTIFY_DPICHANGED, data, 0);
                    }
                    return TRUE;
                },
                nWParam);
            SetWindowPos(nullptr, reinterpret_cast<RECT*> (nLParam), SWP_NOACTIVATE | SWP_NOZORDER);
            bHandled = base_class_t::ProcessWindowMessage(hWnd, nMessage, nWParam, nLParam, nLResult, nMsgMapID);
        }
        break;

        default:
        {
            bHandled = base_class_t::ProcessWindowMessage(hWnd, nMessage, nWParam, nLParam, nLResult, nMsgMapID);
        }
    }

    return bHandled;
}


/**
 * WM_CREATE message handler.
 *
 * Parameters:
 * None.
 *
 * Returns:
 * To continue creation of the window returns 0. To destroy the window returns -1.
 */
LRESULT top_window_impl::createHandler()
{
    LRESULT nResult = base_class_t::createHandler();
    if (!nResult)
    {
        HICON hIcon;
        if (SUCCEEDED(LoadIconMetric(
            ATL::_AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE(IDR_THE_APPLICATION), LIM_SMALL, &hIcon)))
        {
            ::SendMessage(m_hWnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM> (hIcon));
            m_smallIcon.reset(hIcon);
        }
        if (SUCCEEDED(LoadIconMetric(
            ATL::_AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE(IDR_THE_APPLICATION), LIM_LARGE, &hIcon)))
        {
            ::SendMessage(m_hWnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM> (hIcon));
            m_defaultIcon.reset(hIcon);
        }
        m_menu.reset(LoadMenu(ATL::_AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE(IDR_THE_APPLICATION)));
        if (m_menu)
        {
            HMENU hMenu = GetMenu();
            SetMenu(m_menu.get());
            if (nullptr != hMenu)
            {
                ::DestroyMenu(hMenu);
            }
        }
    }
    return nResult;
}


#pragma region window position
/**
 * Repositions this window in concordance with the specified 'WINDOWPLACEMENT' structure..
 *
 * Parameters:
 * >pData
 * Window position information.
 *
 * Returns:
 * If the function succeeds, the return value is true. If the function fails, the return value is false.
 */
bool top_window_impl::loadWindowPlacement(_In_ const WINDOWPLACEMENT* pData)
{
    float fDPIX;
    float fDPIY;
    bool bResult = SUCCEEDED(getDpiForMonitor(m_hWnd, &fDPIX, &fDPIY));
    if (bResult)
    {
        // Recalculate window coordinates taking into account multiple display setup.
        HMONITOR hMonitor = MonitorFromRect(&(pData->rcNormalPosition), MONITOR_DEFAULTTONEAREST);
        MONITORINFO monitorInfo;
        monitorInfo.cbSize = sizeof(MONITORINFO);
        GetMonitorInfo(hMonitor, &monitorInfo);
        LONG nWidth = getRectWidth(pData->rcNormalPosition);
        LONG nHeight = getRectHeight(pData->rcNormalPosition);
        WINDOWPLACEMENT wp;
#if defined(_WIN64)
        size_t nNumberOfReps = sizeof(WINDOWPLACEMENT) >> 3;
        __movsq(reinterpret_cast<unsigned __int64*> (&wp),
                reinterpret_cast<const unsigned __int64*> (pData),
                nNumberOfReps);
        size_t nOffset = nNumberOfReps << 3;
        nNumberOfReps = sizeof(WINDOWPLACEMENT) & 7;
#else
        size_t nNumberOfReps = sizeof(WINDOWPLACEMENT) >> 2;
        __movsd(reinterpret_cast<unsigned long*> (&wp),
                reinterpret_cast<const unsigned long*> (pData),
                nNumberOfReps);
        size_t nOffset = nNumberOfReps << 2;
        nNumberOfReps = sizeof(WINDOWPLACEMENT) & 3;
#endif
        __movsb(reinterpret_cast<unsigned char*> (&wp) + nOffset,
                (reinterpret_cast<const unsigned char*> (pData)) + nOffset,
                nNumberOfReps);
        wp.rcNormalPosition.left =
            max(monitorInfo.rcWork.left, min(monitorInfo.rcWork.right - nWidth, wp.rcNormalPosition.left));
        wp.rcNormalPosition.top =
            max(monitorInfo.rcWork.top, min(monitorInfo.rcWork.bottom - nHeight, wp.rcNormalPosition.top));
        wp.rcNormalPosition.right = wp.rcNormalPosition.left + static_cast<LONG> (logicalToPhysical(nWidth, fDPIX));
        wp.rcNormalPosition.bottom = wp.rcNormalPosition.top + static_cast<LONG> (logicalToPhysical(nHeight, fDPIY));
        wp.length = sizeof(WINDOWPLACEMENT);
        wp.flags = 0;
        SetWindowPlacement(&wp);
    }
    return bResult;
}


/**
 * Stores this window position in the specified 'WINDOWPLACEMENT' structure.
 *
 * Parameters:
 * >pData
 * Pointer to 'WINDOWPLACEMENT' structure to get this window placement.
 *
 * Returns:
 * N/A.
 */
void top_window_impl::storeWindowPlacement(_Out_ WINDOWPLACEMENT* pData) const
{
    pData->length = sizeof(WINDOWPLACEMENT);
    float fDPIX;
    float fDPIY;
    if (SUCCEEDED(getDpiForMonitor(m_hWnd, &fDPIX, &fDPIY)) && GetWindowPlacement(pData))
    {
        pData->rcNormalPosition.right =
            pData->rcNormalPosition.left +
            static_cast<LONG> (physicalToLogical(getRectWidth(pData->rcNormalPosition), fDPIX));
        pData->rcNormalPosition.bottom =
            pData->rcNormalPosition.top +
            static_cast<LONG> (physicalToLogical(getRectHeight(pData->rcNormalPosition), fDPIY));
    }
}
#pragma endregion store/load window position
#pragma endregion top_window_impl implementation
ATLADD_END
