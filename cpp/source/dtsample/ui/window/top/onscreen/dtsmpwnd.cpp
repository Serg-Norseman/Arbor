#define OEMRESOURCE
#include "resource.h"
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
 * >parent
 * Handle to the parent window.
 *
 * Returns:
 * Handle to this window.
 */
_Check_return_ HWND desktop_sample_window::create(_In_ const HWND parent)
{
    typedef ATL::CWinTraits<WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL, WS_EX_CONTROLPARENT>
        style_traits_t;

    HWND hwnd = nullptr;
    float dpiX;
    float dpiY;
    if (SUCCEEDED(getDpiForMonitor(parent, &dpiX, &dpiY)))
    {
        RECT rect;
        rect.left = 0;
        rect.right = static_cast<LONG> (logicalToPhysical(640.0f, dpiX));
        rect.top = 0;
        rect.bottom = static_cast<LONG> (logicalToPhysical(480.0f, dpiY));
        hwnd = Create(
            parent,
            ATL::_U_RECT {rect},
            nullptr,
            style_traits_t::GetWndStyle(0),
            style_traits_t::GetWndExStyle(0),
            0U,
            nullptr);
        if (hwnd)
        {
            if (!loadWindowPlacement())
            {
                CenterWindow();
            }
            m_visualSize = D2D1::SizeU(rect.right >> 1, rect.bottom >> 1);
        }
    }
    return hwnd;
}


#pragma region messages routing
/**
 * Handler of ATL message map (almost the WindowProc).
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
BOOL desktop_sample_window::ProcessWindowMessage(
    _In_ HWND hwnd,
    _In_ UINT message,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    _Inout_ LRESULT& lResult,
    _In_ DWORD msgMapID)
{
    BOOL handled;
    if (m_taskbarButtonCreatedMessage == message)
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
        handled = TRUE;
    }
    else
    {
        switch (message)
        {
            case WM_COMMAND:
            {
                switch (HIWORD(wParam))
                {
                    case 0:
                    case 1:
                    {
                        // Pass both menu items and accelerators (1) to the 'commandHandler' method.
                        handled = commandHandler(LOWORD(wParam));
                        if (handled)
                        {
                            lResult = 0;
                        }
                    }
                    break;

                    default:
                    {
                        handled = FALSE;
                    }
                    break;
                }

                if (!handled)
                {
                    handled = base_class_t::ProcessWindowMessage(hwnd, message, wParam, lParam, lResult, msgMapID);
                }
            }
            break;

            case WM_SETCURSOR:
            {
                if (m_appStartingCursor && (HTCLIENT == LOWORD(lParam)))
                {
                    handled = isAppStartingCursorMustBeSet();
                    if (handled)
                    {
                        SetCursor(m_appStartingCursor);
                        lResult = TRUE;
                    }
                }
                else
                {
                    handled = base_class_t::ProcessWindowMessage(hwnd, message, wParam, lParam, lResult, msgMapID);
                }
            }
            break;

            case WM_HSCROLL:
            case WM_VSCROLL:
            {
                HWND visualHWND;
                if (m_visual && (S_OK == m_visual->getHWND(&visualHWND)))
                {
                    ::SendNotifyMessage(visualHWND, message, wParam, lParam);
                }
                lResult = 0;
                handled = TRUE;
            }
            break;

            case WM_KEYDOWN:
            {
                switch (wParam)
                {
                    case VK_UP:
                    {
                        SendMessage(WM_VSCROLL, MAKELONG(SB_LINEUP, 0), 0);
                        lResult = 0;
                        handled = TRUE;
                    }
                    break;

                    case VK_DOWN:
                    {
                        SendMessage(WM_VSCROLL, MAKELONG(SB_LINEDOWN, 0), 0);
                        lResult = 0;
                        handled = TRUE;
                    }
                    break;

                    case VK_LEFT:
                    {
                        SendMessage(WM_HSCROLL, MAKELONG(SB_LINELEFT, 0), 0);
                        lResult = 0;
                        handled = TRUE;
                    }
                    break;

                    case VK_RIGHT:
                    {
                        SendMessage(WM_HSCROLL, MAKELONG(SB_LINERIGHT, 0), 0);
                        lResult = 0;
                        handled = TRUE;
                    }
                    break;

                    case VK_PRIOR:
                    {
                        SendMessage(WM_VSCROLL, MAKELONG(SB_PAGEUP, 0), 0);
                        lResult = 0;
                        handled = TRUE;
                    }
                    break;

                    case VK_NEXT:
                    {
                        SendMessage(WM_VSCROLL, MAKELONG(SB_PAGEDOWN, 0), 0);
                        lResult = 0;
                        handled = TRUE;
                    }
                    break;

                    case VK_HOME:
                    {
                        SendMessage(WM_VSCROLL, MAKELONG(SB_TOP, 0), 0);
                        lResult = 0;
                        handled = TRUE;
                    }
                    break;

                    case VK_END:
                    {
                        SendMessage(WM_VSCROLL, MAKELONG(SB_BOTTOM, 0), 0);
                        lResult = 0;
                        handled = TRUE;
                    }
                    break;

                    default:
                    {
                        handled = base_class_t::ProcessWindowMessage(hwnd, message, wParam, lParam, lResult, msgMapID);
                    }
                }
            }
            break;

            case WM_GETDLGCODE:
            {
                if ((VK_UP == wParam) || (VK_DOWN == wParam))
                {
                    lResult = DLGC_WANTARROWS;
                    handled = TRUE;
                }
                else
                {
                    handled = base_class_t::ProcessWindowMessage(hwnd, message, wParam, lParam, lResult, msgMapID);
                }
            }
            break;

            default:
            {
                handled = base_class_t::ProcessWindowMessage(hwnd, message, wParam, lParam, lResult, msgMapID);
            }
        }
    }

    return handled;
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
    typedef ATL::CWinTraits<WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE, WS_EX_NOPARENTNOTIFY> style_traits_t;
    // This window and the graph window (`m_visual`'s HWND) are owned by different threads; nobody needs a deadlock,
    // therefore 'WS_EX_NOPARENTNOTIFY' is required.

    LRESULT result = base_class_t::createHandler();
    if (!result)
    {
        updateWindowText(nullptr);
        m_appStartingCursor = static_cast<HCURSOR> (
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
    return result;
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
    size_t size = m_threads.size();
    if (size)
    {
        // Wait for the threads/events.
        WaitForMultipleObjectsEx(static_cast<DWORD> (size), m_threads.data(), TRUE, INFINITE, FALSE);
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
 * >id
 * Command/control identifiers.
 *
 * Returns:
 * true value if the method has handled this notification; false otherwise.
 */
_Check_return_ bool desktop_sample_window::commandHandler(_In_ const UINT id)
{
    bool result = true;
    switch (id)
    {
        case ID_APP_EXIT:
        {
            DestroyWindow();
        }
        break;

        case ID_APP_ABOUT:
        {
            STLADD string_unique_ptr_t description = MISCUTIL version_info::getApplicationExeDescription();
            if (description)
            {
                STLADD string_unique_ptr_t productName = MISCUTIL version_info::getApplicationProductName();
                if (productName)
                {
                    STLADD wostringstream stream;
                    stream << *productName << TEXT(": ") << *description;
                    STLADD string_unique_ptr_t version = MISCUTIL version_info::getApplicationExeVersion();
                    if (version)
                    {
                        stream << TEXT('\n') << *version;
                        ::MessageBox(m_hWnd, stream.str().c_str(), description->c_str(), MB_OK | MB_ICONINFORMATION);
                    }
                }
            }
        }
        break;

        case ID_REFRESH:
        {
            if (WAIT_OBJECT_0 == WaitForSingleObject(m_visualCreated.get(), 0))
            {
                if (SUCCEEDED(m_visual->clear()))
                {
                    addDataToTheGraph();
                }
            }
        }
        break;

        case ID_ZOOMIN:
        {
            float dpiX;
            float dpiY;
            if (SUCCEEDED(getDpiForMonitor(m_hWnd, &dpiX, &dpiY)))
            {
                m_visualSize.width += static_cast<UINT> (logicalToPhysical(m_zoomFactor, dpiX));
                m_visualSize.height += static_cast<UINT> (logicalToPhysical(m_zoomFactor, dpiY));
                resizeVisual();
            }
        }
        break;

        case ID_ZOOMOUT:
        {
            float dpiX;
            float dpiY;
            if (SUCCEEDED(getDpiForMonitor(m_hWnd, &dpiX, &dpiY)))
            {
                m_visualSize.width = max(
                    static_cast<UINT> (GetSystemMetrics(SM_CXMIN)),
                    m_visualSize.width - static_cast<UINT> (logicalToPhysical(m_zoomFactor, dpiX)));
                m_visualSize.height = max(
                    static_cast<UINT> (GetSystemMetrics(SM_CYMIN)),
                    m_visualSize.height - static_cast<UINT> (logicalToPhysical(m_zoomFactor, dpiY)));
                resizeVisual();
            }
        }
        break;

        default:
        {
            result = false;
        }
    }
    return result;
}


/**
 * Changes window text.
 *
 * Parameters:
 * >addText
 * Variable part of the window text.
 *
 * Returns:
 * N/A.
 */
void desktop_sample_window::updateWindowText(_In_opt_ const STLADD string_type* addText)
{
    STLADD string_unique_ptr_t text = MISCUTIL version_info::getApplicationExeDescription();
    if (!text)
    {
        text = std::make_unique<STLADD string_type>(TEXT("Arbor GVT desktop sample"));
    }
    size_t size = text->size() + (addText ? addText->size() + 3 : 0) + 1;
    STLADD t_char_unique_ptr_t windowText {new TCHAR[size]};
    if (windowText)
    {
        if (addText)
        {
            _stprintf_s(windowText.get(), size, TEXT("%s - %s"), addText->c_str(), text->c_str());
        }
        else
        {
            _stprintf_s(windowText.get(), size, TEXT("%s"), text->c_str());
        }
        SetWindowText(windowText.get());
    }
}


/**
 * Handles exit of the specified thread.
 *
 * Parameters:
 * >object
 * Handle to an object owned by this window and which state has become signaled now.
 *
 * Returns:
 * N/A.
 */
void desktop_sample_window::handleThreadTermination(_In_ const HANDLE object)
{
    /*
     * This method is called ONLY by the 'wWinMain' function who calls the method only after the
     * 'MsgWaitForMultipleObjectsEx' was caused to return by some of the threads become signaled. This is the 'object'.
     * So there's necessity to check thread object state. It IS signaled.
     *
     * At first remove the handle from the controlled threads container.
     */
    auto objIt = std::find(m_threads.begin(), m_threads.end(), object);
    m_threads.erase(objIt);
    if (m_visualCreated.get() == object)
    {
        // The graph visual got an HWND. We can enable some graph-window-aware controls here, for example.
        addDataToTheGraph();
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
        float dpiX;
        float dpiY;
        if (SUCCEEDED(getDpiForMonitor(m_hWnd, &dpiX, &dpiY)))
        {
            RECT rect;
            GetClientRect(&rect);
            D2D1_SIZE_F size = D2D1::SizeF(
                logicalToPhysical(m_visualSize.width, dpiX), logicalToPhysical(m_visualSize.height, dpiY));
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
            else
            {
                si.fMask = SIF_POS;
                GetScrollInfo(SB_HORZ, &si);
                ::SendNotifyMessage(visualHWND, WM_HSCROLL, MAKELONG(SB_THUMBTRACK, si.nPos), 0);
                si.fMask = SIF_PAGE | SIF_RANGE;
            }
            SetScrollInfo(SB_HORZ, &si, TRUE);
            si.nPage = rect.bottom;
            si.nMax = max(rect.bottom, origin.y + static_cast<LONG> (size.height)) - 1;
            if (si.nMax < (static_cast<int> (si.nPage)))
            {
                ::SendNotifyMessage(visualHWND, WM_VSCROLL, MAKELONG(SB_TOP, 0), 0);
            }
            else
            {
                si.fMask = SIF_POS;
                GetScrollInfo(SB_VERT, &si);
                ::SendNotifyMessage(visualHWND, WM_VSCROLL, MAKELONG(SB_THUMBTRACK, si.nPos), 0);
                si.fMask = SIF_PAGE | SIF_RANGE;
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
            ::InvalidateRect(visualHWND, nullptr, FALSE);
        }
    }
}


/**
 * Adds sample data to the graph.
 * This method uses `m_visual` GUI, therefore this method must be called only after the graph window GUI was
 * initialized.
 *
 * Parameters:
 * None.
 *
 * Returns:
 * N/A.
 */
void desktop_sample_window::addDataToTheGraph()
{
    using namespace std::string_literals;
    try
    {
        HRESULT hr = m_visual->addEdge(TEXT("vertex.1"s), TEXT("vertex.1.2"s), 1.0f);
        if (SUCCEEDED(hr))
        {
            hr = m_visual->addEdge(TEXT("vertex.1"s), TEXT("vertex.1.3"s), 1.0f);
            if (SUCCEEDED(hr))
            {
                hr = m_visual->addEdge(TEXT("vertex.1.2"s), TEXT("vertex.1.2.4"s), 1.0f);
                if (SUCCEEDED(hr))
                {
                    hr = m_visual->addEdge(
                        TEXT("vertex.1.2"s), TEXT("big vertex.1.2.5 derived\nfrom vertex.1.2"s), 1.0f);
                    if (SUCCEEDED(hr))
                    {
                        m_visual->addEdge(TEXT("vertex.1"s), TEXT("vertex.1.4"s), 1.0f);
                        m_visual->addEdge(TEXT("vertex.1"s), TEXT("vertex.1.5"s), 1.0f);
                        m_visual->addEdge(TEXT("vertex.1.4"s), TEXT("vertex.1.4.1"s), 1.0f);
                        m_visual->addEdge(TEXT("vertex.1.4"s), TEXT("\x221E vertex.1.4.2 \x221E"s), 1.0f);
                        m_visual->addEdge(TEXT("vertex.1.2"s), TEXT("vertex.1.2.5"s), 2.0f);
                        HWND visualHWND;
                        hr = m_visual->getHWND(&visualHWND);
                        if (m_visual && (S_OK == hr))
                        {
                            ::InvalidateRect(visualHWND, nullptr, FALSE);
                        }
                    }
                }
            }
        }
    }
    catch (std::bad_alloc& e)
    {
        ::MessageBoxA(m_hWnd, e.what(), nullptr, MB_OK | MB_ICONERROR);
        DestroyWindow();
    }
}
#pragma endregion desktop_sample_window implementation

ATLADD_END
