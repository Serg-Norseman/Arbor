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
    static STLADD string_unique_ptr_t getApplicationExeVersionInfoStringTableEntry(_In_ LPCTSTR name);
};



class windows_system
{
public:
    _Check_return_ static HANDLE createEvent(_In_opt_z_ LPCWSTR name);
    static void setProcessExplicitAppUserModelID(_In_ PCWSTR appID);
    static void changeWindowMessageFilter(_In_ const HWND hwnd, _In_ const UINT message);
};

MISCUTIL_END
