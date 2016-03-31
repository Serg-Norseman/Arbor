#include "resource.h"
#include "ui\window\appmsg.h"
#include "ui\window\top\twinimpl.h"

ATLADD_BEGIN

#pragma region top_window_impl
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
BOOL top_window_impl::ProcessWindowMessage(
    _In_ HWND hwnd,
    _In_ UINT message,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    _Inout_ LRESULT& lResult,
    _In_ DWORD msgMapID)
{
    BOOL handled;
    switch (message)
    {
        case WM_DISPLAYCHANGE:
        {
            Invalidate();
            lResult = 0;
            handled = TRUE;
        }
        break;

        case WM_DPICHANGED:
        {
            EnumChildWindows(
                hwnd,
                [] (_In_ const HWND hwnd, _In_ const LPARAM data) -> BOOL
                {
                    DWORD ownerThreadId = ::GetWindowThreadProcessId(hwnd, nullptr);
                    if (ownerThreadId == GetCurrentThreadId())
                    {
                        ::SendMessage(hwnd, WM_DPICHANGED, data, 0);
                    }
                    else
                    {
                        // `WM_DPICHANGED` for some reason can't intersect threads bounds. Neither sending nor posting
                        // the message helps. I have to use an user-defined message.
                        ::SendNotifyMessage(hwnd, WM_NOTIFY_DPICHANGED, data, 0);
                    }
                    return TRUE;
                },
                wParam);
            SetWindowPos(nullptr, reinterpret_cast<RECT*> (lParam), SWP_NOACTIVATE | SWP_NOZORDER);
            handled = base_class_t::ProcessWindowMessage(hwnd, message, wParam, lParam, lResult, msgMapID);
        }
        break;

        default:
        {
            handled = base_class_t::ProcessWindowMessage(hwnd, message, wParam, lParam, lResult, msgMapID);
        }
    }

    return handled;
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
    LRESULT result = base_class_t::createHandler();
    if (!result)
    {
        HICON icon;
        if (SUCCEEDED(LoadIconMetric(
            ATL::_AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE(IDR_THE_APPLICATION), LIM_SMALL, &icon)))
        {
            ::SendMessage(m_hWnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM> (icon));
            m_smallIcon.reset(icon);
        }
        if (SUCCEEDED(LoadIconMetric(
            ATL::_AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE(IDR_THE_APPLICATION), LIM_LARGE, &icon)))
        {
            ::SendMessage(m_hWnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM> (icon));
            m_defaultIcon.reset(icon);
        }
        m_menu.reset(LoadMenu(ATL::_AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE(IDR_THE_APPLICATION)));
        if (m_menu)
        {
            HMENU menu = GetMenu();
            SetMenu(m_menu.get());
            if (nullptr != menu)
            {
                ::DestroyMenu(menu);
            }
        }
    }
    return result;
}


#pragma region window position
/**
 * Repositions this window in concordance with the specified 'WINDOWPLACEMENT' structure..
 *
 * Parameters:
 * >data
 * Window position information.
 *
 * Returns:
 * If the function succeeds, the return value is true. If the function fails, the return value is false.
 */
bool top_window_impl::loadWindowPlacement(_In_ const WINDOWPLACEMENT* data)
{
    float dpiX;
    float dpiY;
    bool result = SUCCEEDED(getDpiForMonitor(m_hWnd, &dpiX, &dpiY));
    if (result)
    {
        // Recalculate window coordinates taking into account multiple display setup.
        HMONITOR monitor = MonitorFromRect(&(data->rcNormalPosition), MONITOR_DEFAULTTONEAREST);
        MONITORINFO monitorInfo;
        monitorInfo.cbSize = sizeof(MONITORINFO);
        GetMonitorInfo(monitor, &monitorInfo);
        LONG width = getRectWidth(data->rcNormalPosition);
        LONG height = getRectHeight(data->rcNormalPosition);
        WINDOWPLACEMENT wp;
#if defined(_WIN64)
        size_t numberOfReps = sizeof(WINDOWPLACEMENT) >> 3;
        __movsq(reinterpret_cast<unsigned __int64*> (&wp),
                reinterpret_cast<const unsigned __int64*> (data),
                numberOfReps);
        size_t offset = numberOfReps << 3;
        numberOfReps = sizeof(WINDOWPLACEMENT) & 7;
#else
        size_t numberOfReps = sizeof(WINDOWPLACEMENT) >> 2;
        __movsd(reinterpret_cast<unsigned long*> (&wp),
                reinterpret_cast<const unsigned long*> (data),
                numberOfReps);
        size_t offset = numberOfReps << 2;
        numberOfReps = sizeof(WINDOWPLACEMENT) & 3;
#endif
        __movsb(reinterpret_cast<unsigned char*> (&wp) + offset,
                (reinterpret_cast<const unsigned char*> (data)) + offset,
                numberOfReps);
        wp.rcNormalPosition.left =
            max(monitorInfo.rcWork.left, min(monitorInfo.rcWork.right - width, wp.rcNormalPosition.left));
        wp.rcNormalPosition.top =
            max(monitorInfo.rcWork.top, min(monitorInfo.rcWork.bottom - height, wp.rcNormalPosition.top));
        wp.rcNormalPosition.right = wp.rcNormalPosition.left + static_cast<LONG> (logicalToPhysical(width, dpiX));
        wp.rcNormalPosition.bottom = wp.rcNormalPosition.top + static_cast<LONG> (logicalToPhysical(height, dpiY));
        wp.length = sizeof(WINDOWPLACEMENT);
        wp.flags = 0;
        SetWindowPlacement(&wp);
    }
    return result;
}


/**
 * Stores this window position in the specified 'WINDOWPLACEMENT' structure.
 *
 * Parameters:
 * >data
 * Pointer to 'WINDOWPLACEMENT' structure to get this window placement.
 *
 * Returns:
 * N/A.
 */
void top_window_impl::storeWindowPlacement(_Out_ WINDOWPLACEMENT* data) const
{
    data->length = sizeof(WINDOWPLACEMENT);
    float dpiX;
    float dpiY;
    if (SUCCEEDED(getDpiForMonitor(m_hWnd, &dpiX, &dpiY)) && GetWindowPlacement(data))
    {
        data->rcNormalPosition.right =
            data->rcNormalPosition.left +
            static_cast<LONG> (physicalToLogical(getRectWidth(data->rcNormalPosition), dpiX));
        data->rcNormalPosition.bottom =
            data->rcNormalPosition.top +
            static_cast<LONG> (physicalToLogical(getRectHeight(data->rcNormalPosition), dpiY));
    }
}
#pragma endregion store/load window position
#pragma endregion top_window_impl implementation

ATLADD_END
