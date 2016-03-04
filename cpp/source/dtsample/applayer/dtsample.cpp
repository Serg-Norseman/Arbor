#include "resource.h"
#include "service\functype.h"
#include "service\miscutil.h"
#include "service\strgutil.h"
#include "ui\window\top\onscreen\dtsmpwnd.h"
#include <Windows.h>

#pragma comment(lib, "comctl32.Lib")

int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR pszCmdLine, _In_ int nShowCmd)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(pszCmdLine);
    UNREFERENCED_PARAMETER(nShowCmd);

    ATL::_AtlBaseModule.SetResourceInstance(hInstance);
    string_util::pointer_t pStringUtil = string_util::getInstance();
    pStringUtil->setModuleHandleWithResource(hInstance);

    MSG msg;
    msg.wParam = 0;
    // WAM requires COINIT_APARTMENTTHREADED (see Hilo sample documentation)! Using 'COINIT_MULTITHREADED' can end up
    // with unpredictable behaviour.
    if (SUCCEEDED(::CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
    {
        // May be 'InitCommonControlsEx' is required here to use visual styles with scroll bars.

        SetCurrentProcessExplicitAppUserModelID(L"Serg-Norseman.Arbor.dtsample");
        UINT nTaskbarButtonCreatedMessage = RegisterWindowMessage(TEXT("TaskbarButtonCreated"));
        auto window = std::make_unique<ATLADD desktop_sample_window>(nTaskbarButtonCreatedMessage);
        if (window)
        {
            HWND hWnd = window->create(::GetDesktopWindow());
            if (nullptr != hWnd)
            {
                MISCUTIL windows_system::changeWindowMessageFilter(hWnd, nTaskbarButtonCreatedMessage);
                window->ShowWindow(SW_SHOW);

                HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_THE_APPLICATION));

                while (true)
                {
                    const ATLADD desktop_sample_window::handles_type* pThreads = window->getThreadsToWaitFor();
                    const ATLADD desktop_sample_window::handles_type::size_type nSize = pThreads->size();
                    /*
                     * 'MsgWaitForMultipleObjectsEx' must use QS_ALLINPUT, not QS_ALLEVENTS. The first one has
                     * QS_SENDMESSAGE mask -- it allows to receive messages from another threads or applications.
                     */
                    DWORD nWait = MsgWaitForMultipleObjectsEx(
                        static_cast<DWORD> (nSize),
                        std::data(*pThreads),
                        INFINITE,
                        QS_ALLINPUT,
                        MWMO_INPUTAVAILABLE);
                    if (WAIT_OBJECT_0 + nSize == nWait)
                    {
                        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
                        {
                            if (WM_QUIT == msg.message)
                            {
                                break;
                            }
                            if (!(TranslateAccelerator(hWnd, hAccelTable, &msg) ||
                                window->preTranslateMessage(&msg)))
                            {
                                ::TranslateMessage(&msg);
                                ::DispatchMessage(&msg);
                            }
                        }
                    }
                    else
                    {
                        window->handleThreadTermination(pThreads->at(nWait - WAIT_OBJECT_0));
                    }
                }

                /*
                 * Destroy window explicitly (if it wasn't already) before window C++ object will be deleted -- this is
                 * requirement of the ATL (see destructor of the ATL::CWindowImplRoot<T> for the Debug configuration).
                 */
                if (window->IsWindow())
                {
                    window->DestroyWindow();
                }
            }
            // Release window C++ object before call to the 'CoUninitialize'.
            window.reset();
        }
        ::CoUninitialize();
    }

    return static_cast<int> (msg.wParam);
}
