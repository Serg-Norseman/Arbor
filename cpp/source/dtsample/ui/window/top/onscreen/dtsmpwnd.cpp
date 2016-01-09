#define OEMRESOURCE
#include "dlllayer\arbor.h"
#include "resource.h"
#include "service\miscutil.h"
#include "service\stladdon.h"
#include "service\winapi\directx\dx.h"
#include "ui\window\top\onscreen\dtsmpwnd.h"
#include <memory>
#include <wincodec.h>

#pragma comment(lib, "arborgvt.Lib")

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
    typedef ATL::CWinTraits<WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, WS_EX_CONTROLPARENT> style_traits_t;

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
    LRESULT nResult = base_class_t::createHandler();
    if (!nResult)
    {
        updateWindowText(nullptr);
        m_hAppStartingCursor = static_cast<HCURSOR> (
            LoadImage(nullptr, MAKEINTRESOURCE(OCR_APPSTARTING), IMAGE_CURSOR, 0, 0, LR_SHARED));
        // Create graph rendering window asynchronously.
        ATLADD com_ptr<IArborVisual> visual {};
        HRESULT hr = createArborVisual(visual.getAddressOf());
        if (SUCCEEDED(hr))
        {
            HWND hwnd;
            hr = visual->createWindow(&hwnd);
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
    // TODO: destroy here child window rendering the graph ('cos it's owned by other thread).
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
    /*
     * This method is called ONLY by the 'wWinMain' function who calls the method only after the
     * 'MsgWaitForMultipleObjectsEx' was caused to return by some of the threads become signaled. This is the 'hObject'.
     * So there's necessity to check thread object state. It IS signaled.
     *
     * At first remove the handle from the controlled threads container.
     */
    auto objIt = std::find(m_threads.begin(), m_threads.end(), hObject);
    m_threads.erase(objIt);
}
#pragma endregion desktop_sample_window implementation
ATLADD_END
