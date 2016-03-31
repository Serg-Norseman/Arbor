#include "resource.h"
#include "service\functype.h"
#include "service\miscutil.h"
#include "service\strgutil.h"
#include "ui\window\top\onscreen\dtsmpwnd.h"
#include <Windows.h>

#pragma comment(lib, "comctl32.Lib")

int APIENTRY wWinMain(
    _In_ HINSTANCE instance, _In_opt_ HINSTANCE prevInstance, _In_ LPWSTR cmdLine, _In_ int showCmd)
{
    UNREFERENCED_PARAMETER(prevInstance);
    UNREFERENCED_PARAMETER(cmdLine);
    UNREFERENCED_PARAMETER(showCmd);

    ATL::_AtlBaseModule.SetResourceInstance(instance);
    string_util::pointer_t stringUtil = string_util::getInstance();
    stringUtil->setModuleHandleWithResource(instance);

    MSG msg;
    msg.wParam = 0;
    // WAM requires COINIT_APARTMENTTHREADED (see Hilo sample documentation)! Using 'COINIT_MULTITHREADED' can end up
    // with unpredictable behaviour.
    if (SUCCEEDED(::CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
    {
        // May be 'InitCommonControlsEx' is required here to use visual styles with scroll bars.

        SetCurrentProcessExplicitAppUserModelID(L"Serg-Norseman.Arbor.dtsample");
        UINT taskbarButtonCreatedMessage = RegisterWindowMessage(TEXT("TaskbarButtonCreated"));
        auto window = std::make_unique<ATLADD desktop_sample_window>(taskbarButtonCreatedMessage);
        if (window)
        {
            HWND hwnd = window->create(::GetDesktopWindow());
            if (nullptr != hwnd)
            {
                MISCUTIL windows_system::changeWindowMessageFilter(hwnd, taskbarButtonCreatedMessage);
                window->ShowWindow(SW_SHOW);

                HACCEL accelTable = LoadAccelerators(instance, MAKEINTRESOURCE(IDR_THE_APPLICATION));

                while (true)
                {
                    const ATLADD desktop_sample_window::handles_type* threads = window->getThreadsToWaitFor();
                    const ATLADD desktop_sample_window::handles_type::size_type size = threads->size();
                    /*
                     * 'MsgWaitForMultipleObjectsEx' must use QS_ALLINPUT, not QS_ALLEVENTS. The first one has
                     * QS_SENDMESSAGE mask -- it allows to receive messages from another threads or applications.
                     */
                    DWORD wait = MsgWaitForMultipleObjectsEx(
                        static_cast<DWORD> (size),
                        std::data(*threads),
                        INFINITE,
                        QS_ALLINPUT,
                        MWMO_INPUTAVAILABLE);
                    if (WAIT_OBJECT_0 + size == wait)
                    {
                        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
                        {
                            if (WM_QUIT == msg.message)
                            {
                                break;
                            }
                            if (!(TranslateAccelerator(hwnd, accelTable, &msg) ||
                                window->preTranslateMessage(&msg)))
                            {
                                ::TranslateMessage(&msg);
                                ::DispatchMessage(&msg);
                            }
                        }
                    }
                    else
                    {
                        window->handleThreadTermination(threads->at(wait - WAIT_OBJECT_0));
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
