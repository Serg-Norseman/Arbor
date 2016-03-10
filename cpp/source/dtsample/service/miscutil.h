#pragma once
#include "ns\miscutil.h"
#include "service\stladdon.h"
#include <memory>
#include <vector>
#include <Windows.h>

MISCUTIL_BEGIN

class version_info
{
public:
    static std::vector<WORD> getApplicationExeVersionAsVector();
    static STLADD string_unique_ptr_t getApplicationExeVersion();
    static STLADD string_unique_ptr_t getApplicationExeDescription();
    static STLADD string_unique_ptr_t getApplicationExeInternalName();
    static STLADD string_unique_ptr_t getApplicationProductName();


private:
    static STLADD string_unique_ptr_t getApplicationExeVersionInfoStringTableEntry(_In_ LPCTSTR pszName);
};



class windows_system
{
public:
    _Check_return_ static HANDLE createEvent(_In_opt_z_ LPCWSTR pszName);
    static void setProcessExplicitAppUserModelID(_In_ PCWSTR pszAppID);
    static void changeWindowMessageFilter(_In_ const HWND hWnd, _In_ const UINT nMessage);
};

MISCUTIL_END
