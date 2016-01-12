#include "service\functype.h"
#include "service\memorymg.h"
#include "service\miscutil.h"
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
    memory_manager::pointer_t pMemoryManager = memory_manager::getInstance();
    LPTSTR pszImageFileName = pMemoryManager->mAllocChars<LPTSTR>(MAX_PATH);
    if (pszImageFileName)
    {
        DWORD nBytesCopied = GetModuleFileName(nullptr, pszImageFileName, MAX_PATH);
        if (nBytesCopied && (MAX_PATH != nBytesCopied))
        {
            DWORD nVersionInfoSize = GetFileVersionInfoSize(pszImageFileName, &nBytesCopied);
            if (nVersionInfoSize)
            {
                void* pData = pMemoryManager->mAlloc<void*>(nVersionInfoSize);
                if (pData)
                {
                    VS_FIXEDFILEINFO* pFileInfo;
                    UINT nLength;
                    if (GetFileVersionInfo(pszImageFileName, 0, nVersionInfoSize, pData) &&
                        VerQueryValue(pData, TEXT("\\"), reinterpret_cast<void**> (&pFileInfo), &nLength))
                    {
                        result.reserve(4);
                        result.resize(4);
                        result[0] = HIWORD(pFileInfo->dwFileVersionMS);
                        result[1] = LOWORD(pFileInfo->dwFileVersionMS);
                        result[2] = HIWORD(pFileInfo->dwFileVersionLS);
                        result[3] = LOWORD(pFileInfo->dwFileVersionLS);
                    }
                    pMemoryManager->free(pData);
                }
            }
        }
        pMemoryManager->free(pszImageFileName);
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
    STLADD string_unique_ptr_t szResult;
    auto version(getApplicationExeVersionAsVector());
    if (version.size())
    {
        STLADD wostringstream stream;
        std::copy(version.begin(), version.end(), std::ostream_iterator<WORD, wchar_t> {stream, TEXT(".")});
        szResult = std::make_unique<STLADD string_type>(stream.str());
        szResult->erase(szResult->end() - 1);
    }
    return szResult;
}


/**
 * Gets value of the specified entry in the 'StringFileInfo' resource block. The value the method gets is from the
 * first version language the method found in version languages array.
 *
 * Parameters:
 * >pszName
 * Predefined name of an entry in the 'StringFileInfo' resource block ('CompanyName', 'LegalCopyright' and so on).
 *
 * Returns:
 * The entry value.
 */
STLADD string_unique_ptr_t version_info::getApplicationExeVersionInfoStringTableEntry(_In_ LPCTSTR pszName)
{
    STLADD string_unique_ptr_t szResult;

    memory_manager::pointer_t pMemoryManager = memory_manager::getInstance();
    LPTSTR pszPathFileName = pMemoryManager->mAllocChars<LPTSTR>(MAX_PATH);
    if (pszPathFileName)
    {
        DWORD nCharsCopied = GetModuleFileName(nullptr, pszPathFileName, MAX_PATH);
        if (nCharsCopied && (MAX_PATH != nCharsCopied))
        {
            // Form link object name.
            DWORD nVersionInfoSize = GetFileVersionInfoSize(pszPathFileName, &nCharsCopied);
            if (nVersionInfoSize)
            {
                void* pData = pMemoryManager->mAlloc<void*>(nVersionInfoSize);
                if (pData)
                {
                    typedef struct
                    {
                        WORD nLanguage;
                        WORD nCodePage;
                    }
                    lang_and_code_page_t;

                    lang_and_code_page_t* pTranslation;
                    UINT nLength;
                    if (GetFileVersionInfo(pszPathFileName, 0, nVersionInfoSize, pData) &&
                        VerQueryValue(pData,
                                      TEXT("\\VarFileInfo\\Translation"),
                                      reinterpret_cast<void**> (&pTranslation),
                                      &nLength) &&
                        nLength)
                    {
                        TCHAR szStringFileInfoFormat[] = TEXT("\\StringFileInfo\\%04x%04x\\%s");
                        size_t nEntryPathLength = _countof(szStringFileInfoFormat) + _tcscnlen(pszName, 32);
                        LPTSTR pszStringFileInfoEntry = pMemoryManager->mAllocChars<LPTSTR>(nEntryPathLength);
                        if (pszStringFileInfoEntry)
                        {
                            _stprintf_s(pszStringFileInfoEntry,
                                        nEntryPathLength,
                                        szStringFileInfoFormat,
                                        pTranslation[0].nLanguage,
                                        pTranslation[0].nCodePage,
                                        pszName);

                            LPTSTR pszFileDescription;
                            if (VerQueryValue(pData,
                                              pszStringFileInfoEntry,
                                              reinterpret_cast<void**> (&pszFileDescription),
                                              &nLength))
                            {
                                szResult = std::make_unique<STLADD string_type>(pszFileDescription, nLength - 1);
                            }
                            pMemoryManager->free(pszStringFileInfoEntry);
                        }
                    }
                    pMemoryManager->free(pData);
                }
            }
        }
        pMemoryManager->free(pszPathFileName);
    }

    return szResult;
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
 * >pszName
 * Optional name of the new event object.
 *
 * Returns:
 * Handle to the event object. When the return value ain't NULL handle caller must use the 'CloseHandle' to close the
 * handle.
 */
_Check_return_ HANDLE windows_system::createEvent(_In_opt_z_ LPCWSTR pszName)
{
    using create_event_ex_func_t = _Ret_maybenull_ HANDLE (WINAPI *)(
        _In_opt_ LPSECURITY_ATTRIBUTES, _In_opt_ LPCWSTR, _In_ DWORD, _In_ DWORD);
    using create_event_func_t = _Ret_maybenull_ HANDLE (WINAPI *)(
        _In_opt_ LPSECURITY_ATTRIBUTES, _In_ BOOL, _In_ BOOL, _In_opt_ LPCWSTR);

    HANDLE hEvent = nullptr;
    HMODULE hKernel32 = nullptr;
    create_event_ex_func_t pCreateEventEx =
        getFunction<create_event_ex_func_t>(TEXT("kernel32.dll"), "CreateEventExW", hKernel32);
    if (nullptr != pCreateEventEx)
    {
        hEvent = (pCreateEventEx)(nullptr, pszName, CREATE_EVENT_MANUAL_RESET, SYNCHRONIZE | EVENT_MODIFY_STATE);
    }
    else
    {
        create_event_func_t pCreateEvent = getFunction<create_event_func_t>(nullptr, "CreateEventW", hKernel32);
        if (nullptr != pCreateEvent)
        {
            hEvent = (pCreateEvent)(nullptr, TRUE, FALSE, pszName);
        }
    }
    if (nullptr != hKernel32)
    {
        FreeLibrary(hKernel32);
    }
    return hEvent;
}


/**
 * Assigns unique application-defined Application User Model ID (AppUserModelID) that identifies the current process to
 * the taskbar.
 *
 * Parameters:
 * pszAppID
 * >Application User Model ID to assign to the current process.
 *
 * Returns:
 * N/A.
 */
void windows_system::setProcessExplicitAppUserModelID(_In_ PCWSTR pszAppID)
{
    using set_current_process_explicit_app_user_model_id_func_t = HRESULT (STDAPICALLTYPE *)(_In_ PCWSTR);

    HMODULE hShell32 = nullptr;
    set_current_process_explicit_app_user_model_id_func_t pSetter =
        getFunction<set_current_process_explicit_app_user_model_id_func_t>(
        TEXT("shell32.dll"), "SetCurrentProcessExplicitAppUserModelID", hShell32);
    if (nullptr != pSetter)
    {
        (pSetter)(pszAppID);
    }
    if (nullptr != hShell32)
    {
        FreeLibrary(hShell32);
    }
}


/**
 * Adds a message to a specified window's filter. So in case the application is run elevated, we allow the message
 * through.
 *
 * Parameters:
 * >hWnd
 * Handle to the window whose User Interface Privilege Isolation (UIPI) message filter is to be modified.
 * >nMessage
 * The message that the message filter allows through.
 *
 * Returns:
 * N/A.
 */
void windows_system::changeWindowMessageFilter(_In_ const HWND hWnd, _In_ const UINT nMessage)
{
    using change_window_message_filter_ex_func_t = BOOL (WINAPI *)(
        _In_ HWND, _In_ UINT, _In_ DWORD, _Inout_opt_ PCHANGEFILTERSTRUCT);

    HMODULE hUser32 = nullptr;
    change_window_message_filter_ex_func_t pChangeWindowMessageFilterEx =
        getFunction<change_window_message_filter_ex_func_t> (
        TEXT("user32.dll"), "ChangeWindowMessageFilterEx", hUser32);
    if (nullptr != pChangeWindowMessageFilterEx)
    {
        (pChangeWindowMessageFilterEx)(hWnd, nMessage, MSGFLT_ALLOW, nullptr);
    }
    if (nullptr != hUser32)
    {
        FreeLibrary(hUser32);
    }
}
#pragma endregion windows_system implementation
MISCUTIL_END
