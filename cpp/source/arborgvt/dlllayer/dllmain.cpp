#include "targetver.h"
#include <Windows.h>

BOOL APIENTRY DllMain(_In_ HMODULE hModule, _In_ DWORD nReasonForCall, _In_ LPVOID pReserved)
{
    UNREFERENCED_PARAMETER(hModule);
    UNREFERENCED_PARAMETER(pReserved);

    switch (nReasonForCall)
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
