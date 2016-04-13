#include "service/functype.h"
#include "service/miscutil.h"
#include "service/stladdon.h"
#include <shellapi.h>
#include <tchar.h>

#pragma comment(lib, "version.lib")

MISCUTIL_BEGIN

#pragma region version_info
/**
 * Returns application exe module version.
 *
 * Parameters:
 * None.
 *
 * Returns:
 * Application version number as vector of WORDs. The first element is major version number, the second one is minor,
 * the third one is build number and the fourth element is revision.
 */
std::vector<WORD> version_info::getApplicationExeVersionAsVector()
{
    std::vector<WORD> result;
    STLADD t_char_unique_ptr_t imageFileName {new TCHAR[MAX_PATH]};
    DWORD bytesCopied = GetModuleFileName(nullptr, imageFileName.get(), MAX_PATH);
    if (bytesCopied && (MAX_PATH != bytesCopied))
    {
        DWORD versionInfoSize = GetFileVersionInfoSize(imageFileName.get(), &bytesCopied);
        if (versionInfoSize)
        {
            STLADD u_char_unique_ptr_t data {new unsigned char[versionInfoSize]};
            VS_FIXEDFILEINFO* fileInfo;
            UINT length;
            if (GetFileVersionInfo(imageFileName.get(), 0, versionInfoSize, data.get()) &&
                VerQueryValue(data.get(), TEXT("\\"), reinterpret_cast<void**> (&fileInfo), &length))
            {
                result.reserve(4);
                result.resize(4);
                result[0] = HIWORD(fileInfo->dwFileVersionMS);
                result[1] = LOWORD(fileInfo->dwFileVersionMS);
                result[2] = HIWORD(fileInfo->dwFileVersionLS);
                result[3] = LOWORD(fileInfo->dwFileVersionLS);
            }
        }
    }
    return result;
}

/**
 * Returns this application version string in format:
 * mmmmm.nnnnn.ooooo.ppppp.
 * Each of the parts separated by periods can be 0-65535 inclusive-left-to-right, the major, minor, build, and
 * revision parts.
 *
 * Parameters:
 * None.
 *
 * Returns:
 * Application version number string.
 */
STLADD string_unique_ptr_t version_info::getApplicationExeVersion()
{
    STLADD string_unique_ptr_t result {};
#if defined(__ICL)
    auto version(getApplicationExeVersionAsVector());
#else
    auto version {getApplicationExeVersionAsVector()};
#endif
    if (version.size())
    {
        STLADD wostringstream stream {};
        std::copy(version.begin(), version.end(), std::ostream_iterator<WORD, wchar_t> {stream, TEXT(".")});
        result = std::make_unique<STLADD string_type>(stream.str());
        result->erase(result->end() - 1);
    }
    return result;
}


/**
 * Gets value of the specified entry in the 'StringFileInfo' resource block. The value the method gets is from the
 * first version language the method found in version languages array.
 *
 * Parameters:
 * >name
 * Predefined name of an entry in the 'StringFileInfo' resource block ('CompanyName', 'LegalCopyright' and so on).
 *
 * Returns:
 * The entry value.
 */
STLADD string_unique_ptr_t version_info::getApplicationExeVersionInfoStringTableEntry(_In_ LPCTSTR name)
{
    STLADD string_unique_ptr_t result;

    STLADD t_char_unique_ptr_t pathFileName {new TCHAR[MAX_PATH]};
    DWORD charsCopied = GetModuleFileName(nullptr, pathFileName.get(), MAX_PATH);
    if (charsCopied && (MAX_PATH != charsCopied))
    {
        // Form link object name.
        DWORD versionInfoSize = GetFileVersionInfoSize(pathFileName.get(), &charsCopied);
        if (versionInfoSize)
        {
            STLADD u_char_unique_ptr_t data {new unsigned char[versionInfoSize]};
            typedef struct
            {
                WORD language;
                WORD codePage;
            }
            lang_and_code_page_t;

            lang_and_code_page_t* translation;
            UINT length;
            if (GetFileVersionInfo(pathFileName.get(), 0, versionInfoSize, data.get()) &&
                VerQueryValue(
                data.get(), TEXT("\\VarFileInfo\\Translation"), reinterpret_cast<void**> (&translation), &length) &&
                length)
            {
                TCHAR stringFileInfoFormat[] = TEXT("\\StringFileInfo\\%04x%04x\\%s");
                size_t entryPathLength = _countof(stringFileInfoFormat) + _tcscnlen(name, 32);
                STLADD t_char_unique_ptr_t stringFileInfoEntry {new TCHAR[entryPathLength]};
                _stprintf_s(
                    stringFileInfoEntry.get(),
                    entryPathLength,
                    stringFileInfoFormat,
                    translation[0].language,
                    translation[0].codePage,
                    name);

                LPTSTR fileDescription;
                if (VerQueryValue(
                    data.get(), stringFileInfoEntry.get(), reinterpret_cast<void**> (&fileDescription), &length))
                {
                    result = std::make_unique<STLADD string_type>(fileDescription, length - 1);
                }
            }
        }
    }

    return result;
}


/**
 * Gets the "FileDescription" string-value from version-information resource of the application's executable module.
 * It retrieves the FileDescription value for the first version language, that method finds in version languages array.
 *
 * Parameters:
 * None.
 *
 * Returns:
 * Description of the application's executable module. Can be empty pointer.
 */
STLADD string_unique_ptr_t version_info::getApplicationExeDescription()
{
    return getApplicationExeVersionInfoStringTableEntry(TEXT("FileDescription"));
}


/**
 * Gets the "InternalName" string-value from version-information resource of the application's executable module.
 * It retrieves the InternalName value for the first version language, that method finds in version languages array.
 *
 * Parameters:
 * None.
 *
 * Returns:
 * Internal name of the application's executable module. Can be empty pointer.
 */
STLADD string_unique_ptr_t version_info::getApplicationExeInternalName()
{
    return getApplicationExeVersionInfoStringTableEntry(TEXT("InternalName"));
}


/**
 * Gets the "ProductName" string-value from version-information resource of the application's executable module.
 * It retrieves the ProductName value for the first version language, that method finds in version languages array.
 *
 * Parameters:
 * None.
 *
 * Returns:
 * Product name of the application's executable module. Can be empty pointer.
 */
STLADD string_unique_ptr_t version_info::getApplicationProductName()
{
    return getApplicationExeVersionInfoStringTableEntry(TEXT("ProductName"));
}
#pragma endregion version_info implementation

#pragma region windows_system
/**
 * Creates event object.
 * When the 'CreateEventEx' function exists in running environment the method uses it. Otherwise it uses the
 * 'CreateEvent' function.
 * The method creates a nonsignaled manual-reset event object, which requires the use of the ResetEvent function to set
 * the event state to nonsignaled.
 *
 * Parameters:
 * >name
 * Optional name of the new event object.
 *
 * Returns:
 * Handle to the event object. When the return value ain't NULL handle caller must use the 'CloseHandle' to close the
 * handle.
 */
_Check_return_ HANDLE windows_system::createEvent(_In_opt_z_ LPCWSTR name)
{
    using create_event_ex_func_t = _Ret_maybenull_ HANDLE (WINAPI *)(
        _In_opt_ LPSECURITY_ATTRIBUTES, _In_opt_ LPCWSTR, _In_ DWORD, _In_ DWORD);
    using create_event_func_t = _Ret_maybenull_ HANDLE (WINAPI *)(
        _In_opt_ LPSECURITY_ATTRIBUTES, _In_ BOOL, _In_ BOOL, _In_opt_ LPCWSTR);

    HANDLE event = nullptr;
    HMODULE kernel32 = nullptr;
    create_event_ex_func_t createEventEx =
        getFunction<create_event_ex_func_t>(TEXT("kernel32.dll"), "CreateEventExW", kernel32);
    if (nullptr != createEventEx)
    {
        event = (createEventEx)(nullptr, name, CREATE_EVENT_MANUAL_RESET, SYNCHRONIZE | EVENT_MODIFY_STATE);
    }
    else
    {
        create_event_func_t createEvent = getFunction<create_event_func_t>(nullptr, "CreateEventW", kernel32);
        if (nullptr != createEvent)
        {
            event = (createEvent)(nullptr, TRUE, FALSE, name);
        }
    }
    if (nullptr != kernel32)
    {
        FreeLibrary(kernel32);
    }
    return event;
}


/**
 * Assigns unique application-defined Application User Model ID (AppUserModelID) that identifies the current process to
 * the taskbar.
 *
 * Parameters:
 * appID
 * >Application User Model ID to assign to the current process.
 *
 * Returns:
 * N/A.
 */
void windows_system::setProcessExplicitAppUserModelID(_In_ PCWSTR appID)
{
    using set_current_process_explicit_app_user_model_id_func_t = HRESULT (STDAPICALLTYPE *)(_In_ PCWSTR);

    HMODULE shell32 = nullptr;
    set_current_process_explicit_app_user_model_id_func_t setter =
        getFunction<set_current_process_explicit_app_user_model_id_func_t>(
        TEXT("shell32.dll"), "SetCurrentProcessExplicitAppUserModelID", shell32);
    if (nullptr != setter)
    {
        (setter)(appID);
    }
    if (nullptr != shell32)
    {
        FreeLibrary(shell32);
    }
}


/**
 * Adds a message to a specified window's filter. So in case the application is run elevated, we allow the message
 * through.
 *
 * Parameters:
 * >hwnd
 * Handle to the window whose User Interface Privilege Isolation (UIPI) message filter is to be modified.
 * >message
 * The message that the message filter allows through.
 *
 * Returns:
 * N/A.
 */
void windows_system::changeWindowMessageFilter(_In_ const HWND hwnd, _In_ const UINT message)
{
    using change_window_message_filter_ex_func_t = BOOL (WINAPI *)(
        _In_ HWND, _In_ UINT, _In_ DWORD, _Inout_opt_ PCHANGEFILTERSTRUCT);

    HMODULE user32 = nullptr;
    change_window_message_filter_ex_func_t changeWindowMessageFilterEx =
        getFunction<change_window_message_filter_ex_func_t> (
        TEXT("user32.dll"), "ChangeWindowMessageFilterEx", user32);
    if (nullptr != changeWindowMessageFilterEx)
    {
        (changeWindowMessageFilterEx)(hwnd, message, MSGFLT_ALLOW, nullptr);
    }
    if (nullptr != user32)
    {
        FreeLibrary(user32);
    }
}
#pragma endregion windows_system implementation

MISCUTIL_END
