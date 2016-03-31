#include "sdkver.h"
#include <Windows.h>

BOOL APIENTRY DllMain(_In_ HMODULE module, _In_ DWORD reasonForCall, _In_ LPVOID reserved)
{
    UNREFERENCED_PARAMETER(module);
    UNREFERENCED_PARAMETER(reserved);

    switch (reasonForCall)
    {
        case DLL_PROCESS_ATTACH:
        case DLL_PROCESS_DETACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        {
        }
        break;
    }
    return TRUE;
}
