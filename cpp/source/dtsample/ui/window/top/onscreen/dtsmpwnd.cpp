#define OEMRESOURCE
#include "resource.h"
#include "service\memorymg.h"
#include "service\miscutil.h"
#include "service\stladdon.h"
#include "service\winapi\directx\dx.h"
#include "ui\window\appmsg.h"
#include "ui\window\top\onscreen\dtsmpwnd.h"
#include <memory>
#include <wincodec.h>

#pragma comment(lib, "arborgvt.lib")

ATLADD_BEGIN
#pragma region desktop_sample_window
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
_Check_return_ HWND desktop_sample_window::create(_In_ const HWND hParent)
{
    typedef ATL::CWinTraits<WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL, WS_EX_CONTROLPARENT>
        style_traits_t;

    HWND hWnd = nullptr;
    float fDPIX;
    float fDPIY;
    if (SUCCEEDED(getDpiForMonitor(hParent, &fDPIX, &fDPIY)))
    {
        RECT rect;
        rect.left = 0;
        rect.right = static_cast<LONG> (logicalToPhysical(640.0f, fDPIX));
        rect.top = 0;
        rect.bottom = static_cast<LONG> (logicalToPhysical(480.0f, fDPIY));
        hWnd = Create(
            hParent,
            ATL::_U_RECT {rect},
            nullptr,
            style_traits_t::GetWndStyle(0),
            style_traits_t::GetWndExStyle(0),
            0U,
            nullptr);
        if (hWnd)
        {
            if (!loadWindowPlacement())
            {
                CenterWindow();
            }
            m_visualSize = D2D1::SizeU(rect.right >> 1, rect.bottom >> 1);
        }
    }
    return hWnd;
}


#pragma region messages routing
/**
 * Handler of ATL message map (almost the WindowProc).
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
BOOL desktop_sample_window::ProcessWindowMessage(
    _In_ HWND hWnd,
    _In_ UINT nMessage,
    _In_ WPARAM nWParam,
    _In_ LPARAM nLParam,
    _Inout_ LRESULT& nLResult,
    _In_ DWORD nMsgMapID)
{
    BOOL bHandled;
    if (m_nTaskbarButtonCreatedMessage == nMessage)
    {
        if (!m_taskbarList3)
        {
            if (SUCCEEDED(m_taskbarList3.coCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_INPROC_SERVER)))
            {
                if (SUCCEEDED(m_taskbarList3->HrInit()))
                {
                    if (m_threads.size() &&
                        (WAIT_TIMEOUT ==
                        WaitForMultipleObjects(static_cast<DWORD> (m_threads.size()), m_threads.data(), TRUE, 0)))
                    {
                        m_taskbarList3->SetProgressState(m_hWnd, TBPF_INDETERMINATE);
                    }
                }
                else
                {
                    m_taskbarList3.reset();
                }
            }
        }
        bHandled = TRUE;
    }
    else
    {
        switch (nMessage)
        {
            case WM_COMMAND:
            {
                switch (HIWORD(nWParam))
                {
                    case 0:
                    case 1:
                    {
                        // Pass both menu items and accelerators (1) to the 'commandHandler' method.
                        bHandled = commandHandler(LOWORD(nWParam));
                        if (bHandled)
                        {
                            nLResult = 0;
                        }
                    }
                    break;

                    default:
                    {
                        bHandled = FALSE;
                    }
                    break;
                }

                if (!bHandled)
                {
                    bHandled =
                        base_class_t::ProcessWindowMessage(hWnd, nMessage, nWParam, nLParam, nLResult, nMsgMapID);
                }
            }
            break;

            case WM_SETCURSOR:
            {
                if (m_hAppStartingCursor && (HTCLIENT == LOWORD(nLParam)))
                {
                    bHandled = isAppStartingCursorMustBeSet();
                    if (bHandled)
                    {
                        SetCursor(m_hAppStartingCursor);
                        nLResult = TRUE;
                    }
                }
                else
                {
                    bHandled =
                        base_class_t::ProcessWindowMessage(hWnd, nMessage, nWParam, nLParam, nLResult, nMsgMapID);
                }
            }
            break;

            case WM_HSCROLL:
            case WM_VSCROLL:
            {
                HWND visualHWND;
                if (m_visual && (S_OK == m_visual->getHWND(&visualHWND)))
                {
                    ::SendNotifyMessage(visualHWND, nMessage, nWParam, nLParam);
                }
                nLResult = 0;
                bHandled = TRUE;
            }
            break;

            case WM_KEYDOWN:
            {
                switch (nWParam)
                {
                    // Make this window WS_TABSTOP'd to handle the keys.
                    case VK_UP:
                    {
                        SendMessage(WM_VSCROLL, MAKELONG(SB_LINEUP, 0), 0);
                        nLResult = 0;
                        bHandled = TRUE;
                    }
                    break;

                    case VK_DOWN:
                    {
                        SendMessage(WM_VSCROLL, MAKELONG(SB_LINEDOWN, 0), 0);
                        nLResult = 0;
                        bHandled = TRUE;
                    }
                    break;

                    case VK_PRIOR:
                    {
                        SendMessage(WM_VSCROLL, MAKELONG(SB_PAGEUP, 0), 0);
                        nLResult = 0;
                        bHandled = TRUE;
                    }
                    break;

                    case VK_NEXT:
                    {
                        SendMessage(WM_VSCROLL, MAKELONG(SB_PAGEDOWN, 0), 0);
                        nLResult = 0;
                        bHandled = TRUE;
                    }
                    break;

                    case VK_HOME:
                    {
                        SendMessage(WM_VSCROLL, MAKELONG(SB_TOP, 0), 0);
                        nLResult = 0;
                        bHandled = TRUE;
                    }
                    break;

                    case VK_END:
                    {
                        SendMessage(WM_VSCROLL, MAKELONG(SB_BOTTOM, 0), 0);
                        nLResult = 0;
                        bHandled = TRUE;
                    }
                    break;

                    default:
                    {
                        bHandled =
                            base_class_t::ProcessWindowMessage(hWnd, nMessage, nWParam, nLParam, nLResult, nMsgMapID);
                    }
                }
            }
            break;

            case WM_GETDLGCODE:
            {
                if ((VK_UP == nWParam) || (VK_DOWN == nWParam))
                {
                    nLResult = DLGC_WANTARROWS;
                    bHandled = TRUE;
                }
                else
                {
                    bHandled =
                        base_class_t::ProcessWindowMessage(hWnd, nMessage, nWParam, nLParam, nLResult, nMsgMapID);
                }
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
#pragma endregion methods that route all messages


/**
 * WM_CREATE message handler.
 *
 * Parameters:
 * None.
 *
 * Returns:
 * To continue creation of the window returns 0. To destroy the window returns -1.
 */
LRESULT desktop_sample_window::createHandler()
{
    typedef ATL::CWinTraits<WS_CHILD | WS_CLIPSIBLINGS | WS_TABSTOP | WS_VISIBLE, WS_EX_NOPARENTNOTIFY> style_traits_t;
    // This window and the graph window (`m_visual`'s HWND) are owned by different threads; nobody needs a deadlock,
    // therefore 'WS_EX_NOPARENTNOTIFY' is required.

    LRESULT nResult = base_class_t::createHandler();
    if (!nResult)
    {
        updateWindowText(nullptr);
        m_hAppStartingCursor = static_cast<HCURSOR> (
            LoadImage(nullptr, MAKEINTRESOURCE(OCR_APPSTARTING), IMAGE_CURSOR, 0, 0, LR_SHARED));
        // Create graph rendering window asynchronously.
        HRESULT hr = createArborVisual(m_visual.getAddressOf());
        if (SUCCEEDED(hr))
        {
            m_visualCreated.reset(MISCUTIL windows_system::createEvent(nullptr));
            m_threads.push_back(m_visualCreated.get());
            hr = m_visual->createWindow(
                m_hWnd,
                style_traits_t::GetWndStyle(0),
                style_traits_t::GetWndExStyle(0),
                m_visualCreated.get(),
                WM_NOTIFY_DPICHANGED);
        }
    }
    return nResult;
}


/**
 * WM_DESTROY message handler.
 *
 * Parameters:
 * None.
 *
 * Returns:
 * N/A.
 */
void desktop_sample_window::destroyHandler()
{
    /*
     * Stop all threads owned by this window.
     */
    size_t nSize = m_threads.size();
    if (nSize)
    {
        // Wait for the threads/events.
        WaitForMultipleObjectsEx(static_cast<DWORD> (nSize), m_threads.data(), TRUE, INFINITE, FALSE);
        m_threads.clear();
    }
    base_class_t::destroyHandler();

    /*
     * This window is the window, that must terminate owner thread's message loop after its destruction.
     */
    PostQuitMessage(0);
}


/**
 * WM_COMMAND message handler.
 *
 * Parameters:
 * >nId
 * Command/control identifiers.
 *
 * Returns:
 * true value if the method has handled this notification; false otherwise.
 */
_Check_return_ bool desktop_sample_window::commandHandler(_In_ const UINT nId)
{
    bool bResult = true;
    switch (nId)
    {
        case ID_APP_EXIT:
        {
            DestroyWindow();
        }
        break;

        case ID_APP_ABOUT:
        {
            STLADD string_unique_ptr_t szDescription = MISCUTIL version_info::getApplicationExeDescription();
            if (szDescription)
            {
                STLADD string_unique_ptr_t szProductName = MISCUTIL version_info::getApplicationProductName();
                if (szProductName)
                {
                    STLADD wostringstream stream;
                    stream << *szProductName << TEXT(": ") << *szDescription;
                    STLADD string_unique_ptr_t szVersion = MISCUTIL version_info::getApplicationExeVersion();
                    if (szVersion)
                    {
                        stream << std::endl << *szVersion;
                        ::MessageBox(m_hWnd, stream.str().c_str(), szDescription->c_str(), MB_OK | MB_ICONINFORMATION);
                    }
                }
            }
        }
        break;

        case ID_REFRESH:
        {
            ::MessageBox(m_hWnd, TEXT("NYI"), nullptr, 0);
        }
        break;

        case ID_ZOOMIN:
        {
            float fDPIX;
            float fDPIY;
            if (SUCCEEDED(getDpiForMonitor(m_hWnd, &fDPIX, &fDPIY)))
            {
                m_visualSize.width += static_cast<UINT> (logicalToPhysical(m_cZoomFactor, fDPIX));
                m_visualSize.height += static_cast<UINT> (logicalToPhysical(m_cZoomFactor, fDPIY));
                resizeVisual();
            }
        }
        break;

        case ID_ZOOMOUT:
        {
            float fDPIX;
            float fDPIY;
            if (SUCCEEDED(getDpiForMonitor(m_hWnd, &fDPIX, &fDPIY)))
            {
                m_visualSize.width = max(
                    static_cast<UINT> (GetSystemMetrics(SM_CXMIN)),
                    m_visualSize.width - static_cast<UINT> (logicalToPhysical(m_cZoomFactor, fDPIX)));
                m_visualSize.height = max(
                    static_cast<UINT> (GetSystemMetrics(SM_CYMIN)),
                    m_visualSize.height - static_cast<UINT> (logicalToPhysical(m_cZoomFactor, fDPIY)));
                resizeVisual();
            }
        }
        break;

        default:
        {
            bResult = false;
        }
    }
    return bResult;
}


/**
 * Changes window text.
 *
 * Parameters:
 * >pszText
 * Variable part of the window text.
 *
 * Returns:
 * N/A.
 */
void desktop_sample_window::updateWindowText(_In_opt_ const STLADD string_type* pszText)
{
    STLADD string_unique_ptr_t szText = MISCUTIL version_info::getApplicationExeDescription();
    if (!szText)
    {
        szText = std::make_unique<STLADD string_type>(TEXT("Arbor GVT desktop sample"));
    }
    memory_manager::pointer_t pMemoryManager = memory_manager::getInstance();
    size_t nSize = szText->size() + (pszText ? pszText->size() + 3 : 0) + 1;
    STLADD t_char_unique_ptr_t szWindowText {pMemoryManager->mAllocChars<LPTSTR>(nSize)};
    if (szWindowText)
    {
        if (pszText)
        {
            _stprintf_s(szWindowText.get(), nSize, TEXT("%s - %s"), pszText->c_str(), szText->c_str());
        }
        else
        {
            _stprintf_s(szWindowText.get(), nSize, TEXT("%s"), szText->c_str());
        }
        SetWindowText(szWindowText.get());
    }
}


/**
 * Handles exit of the specified thread.
 *
 * Parameters:
 * >hObject
 * Handle to an object owned by this window and which state has become signaled now.
 *
 * Returns:
 * N/A.
 */
void desktop_sample_window::handleThreadTermination(_In_ const HANDLE hObject)
{
    using namespace std::string_literals;
    /*
     * This method is called ONLY by the 'wWinMain' function who calls the method only after the
     * 'MsgWaitForMultipleObjectsEx' was caused to return by some of the threads become signaled. This is the 'hObject'.
     * So there's necessity to check thread object state. It IS signaled.
     *
     * At first remove the handle from the controlled threads container.
     */
    auto objIt = std::find(m_threads.begin(), m_threads.end(), hObject);
    m_threads.erase(objIt);
    if (m_visualCreated.get() == hObject)
    {
        // The graph visual got an HWND. We can enable some graph-window-aware controls here, for example.
        HRESULT hr = m_visual->addEdge(TEXT("vertex.1"s), TEXT("vertex.2"s), 1.5f);
        if (SUCCEEDED(hr))
        {
            hr = m_visual->addEdge(TEXT("vertex.1"s), TEXT("vertex.3"s), 1.75f);
            if (SUCCEEDED(hr))
            {
            }
        }
    }
    if (0 == m_threads.size())
    {
        if (m_taskbarList3)
        {
            m_taskbarList3->SetProgressState(m_hWnd, TBPF_NOPROGRESS);
        }
    }
}


/**
 * Resize the graph window.
 *
 * Parameters:
 * None.
 *
 * Returns:
 * N/A.
 */
void desktop_sample_window::resizeVisual()
{
    HWND visualHWND;
    if (m_visual && (S_OK == m_visual->getHWND(&visualHWND)))
    {
        float fDPIX;
        float fDPIY;
        if (SUCCEEDED(getDpiForMonitor(m_hWnd, &fDPIX, &fDPIY)))
        {
            RECT rect;
            GetClientRect(&rect);
            D2D1_SIZE_F size = D2D1::SizeF(
                logicalToPhysical(m_visualSize.width, fDPIX), logicalToPhysical(m_visualSize.height, fDPIY));
            D2D1_POINT_2L origin = D2D1::Point2L(
                max(0, (rect.right - (static_cast<int> (size.width))) >> 1),
                max(0, (rect.bottom - (static_cast<int> (size.height))) >> 1));
            /*
             * Initialize scroll bars.
             * Member of the SCROLLINFO structure must be initialized with _PHYSICAL_ coordinates.
             */
            SCROLLINFO si;
            si.cbSize = sizeof(si);
            si.fMask = SIF_PAGE | SIF_RANGE;
            si.nMin = 0;
            si.nPage = rect.right;
            si.nMax = max(rect.right, origin.x + static_cast<LONG> (size.width)) - 1;
            if (si.nMax < (static_cast<int> (si.nPage)))
            {
                ::SendNotifyMessage(visualHWND, WM_HSCROLL, MAKELONG(SB_LEFT, 0), 0);
            }
            SetScrollInfo(SB_HORZ, &si, TRUE);
            si.nPage = rect.bottom;
            si.nMax = max(rect.bottom, origin.y + static_cast<LONG> (size.height)) - 1;
            if (si.nMax < (static_cast<int> (si.nPage)))
            {
                ::SendNotifyMessage(visualHWND, WM_VSCROLL, MAKELONG(SB_TOP, 0), 0);
            }
            SetScrollInfo(SB_VERT, &si, TRUE);

            ::SetWindowPos(
                visualHWND,
                nullptr,
                origin.x,
                origin.y,
                static_cast<int> (size.width),
                static_cast<int> (size.height),
                SWP_NOACTIVATE | SWP_NOZORDER);
        }
    }
}
#pragma endregion desktop_sample_window implementation
ATLADD_END
