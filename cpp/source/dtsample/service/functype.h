#pragma once
#include <ShellScalingApi.h>
#include <windef.h>

using path_cch_append_ex_func_t = HRESULT (APIENTRY *)(
    _Inout_updates_(cchPath) PWSTR,
    _In_ size_t cchPath,
    _In_opt_ PCWSTR,
    _In_ unsigned long);
using path_cch_remove_file_spec_func_t = HRESULT (APIENTRY *)(
    _Inout_updates_(_Inexpressible_(cchPath)) PWSTR,
    _In_ size_t cchPath);
using path_cch_find_extension_func_t = HRESULT (APIENTRY *)(
    _In_reads_(_Inexpressible_(cchPath)) PWSTR,
    _In_ size_t cchPath,
    _Outptr_ PWSTR*);
using path_append_func_t = BOOL (STDAPICALLTYPE *)(_Inout_updates_(MAX_PATH) LPWSTR, _In_ LPCWSTR);
using path_remove_file_spec_func_t = BOOL (STDAPICALLTYPE *)(_Inout_ LPWSTR);
using path_find_extension_func_t = LPWSTR (STDAPICALLTYPE *)(_Inout_ LPWSTR);
using get_locale_info_ex_func_t = int (WINAPI *)(
    _In_opt_ LPCWSTR,
    _In_ LCTYPE,
    _Out_writes_opt_(cchData) LPWSTR,
    _In_ int cchData);
using get_locale_info_func_t = int (WINAPI *)(
    _In_ LCID,
    _In_ LCTYPE,
    _Out_writes_opt_(cchData) LPTSTR,
    _In_ int cchData);
using get_dpi_for_monitor_func_t = HRESULT (STDAPICALLTYPE *)(
    _In_ HMONITOR, _In_ MONITOR_DPI_TYPE, _Out_ UINT*, _Out_ UINT*);


/**
 * Gets pointer to a function from a Windows library module.
 * When necessary this function loads the specified module.
 *
 * Parameters:
 * >moduleName
 * Name of module that contains 'functionName'.
 * >functionName
 * Function of type T that is implemented in the 'moduleName'.
 * >lib
 * Handle to the loaded library module. When caller passes a valid handle the method uses it. Otherwise the method
 * returns here handle to the found/loaded library and caller must use 'FreeLibrary' to release it. If caller uses valid
 * 'lib' when call this function the 'moduleName' is ignored.
 *
 * Returns:
 * Pointer to the function.
 */
template<typename T>
_Check_return_ T getFunction(
    _In_opt_z_ LPCTSTR moduleName, _In_z_ LPCSTR functionName, _Inout_ HMODULE& lib)
{
    if (nullptr == lib)
    {
        if (!GetModuleHandleEx(0, moduleName, &lib) && (ERROR_MOD_NOT_FOUND == GetLastError()) && moduleName)
        {
            lib = LoadLibraryEx(moduleName, nullptr, 0);
        }
    }
    if (nullptr != lib)
    {
        return reinterpret_cast<T> (GetProcAddress(lib, functionName));
    }
    else
    {
        return nullptr;
    }
}
