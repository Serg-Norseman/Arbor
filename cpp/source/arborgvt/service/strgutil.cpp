#include "service\functype.h"
#include "service\miscutil.h"
#include "service\stladdon.h"
#include "service\strgutil.h"
#include <regex>
#include <tchar.h>
#include <strsafe.h>
#include <type_traits>

string_util::unique_ptr_t string_util::m_instance;
LPTSTR* string_util::m_ppszEnumDateFormatsProcExData;

// Default character(s) used to separate list items (default for a case when attempt to take it from the regional
// settings failed).
#define DEFAULT_LIST_SEPARATOR (TEXT(", "))
// Maximum number of characters in the LOCALE_SSHORTDATE string (including the terminating null character).
#define LOCALE_SSHORTDATE_MAX_LENGTH (80)

#pragma region string_util
//template STLADD string_unique_ptr_t string_util::formatNumber<size_t>(_In_ const size_t value) const;
//template STLADD string_unique_ptr_t string_util::formatNumber<float>(_In_ const float value) const;
#pragma endregion string_util class template functions instantiation

#pragma region string_util
template <typename T>
class printf_value_trait
{
public:
    static LPCTSTR get()
    {
        return TEXT("%u");
    }
};

template <>
class printf_value_trait<float>
{
public:
    static LPCTSTR get()
    {
        return TEXT("%f");
    }
};


/**
 * Gets the singleton instance of the string_util class.
 *
 * Parameters:
 * None.
 *
 * Returns:
 * Smart pointer to the singleton instance.
 */
string_util::pointer_t string_util::getInstance() noexcept(false)
{
    class initializer
    {
    public:
        initializer()
        {
            m_instance.reset(new string_util {});
        }
    };
    static initializer guard {};
    return m_instance.get();
}


/**
 * Loads a string resource from the m_hModuleWithResource module, copies the string into a buffer,
 * and appends a terminating NULL character.
 *
 * Parameters:
 * >nStringResourceId
 * Specifies the integer identifier of the string to be loaded.
 * >pszAllocatedBuffer
 * Pointer to the buffer to receive the string.
 * >nBufferLength
 * Specifies the size of the buffer, in characters.
 *
 * Returns:
 * true value in case of success, false -- otherwise.
 */
_Success_(return) bool string_util::loadString(
    _In_ UINT nStringResourceId,
    _Out_writes_opt_z_(nBufferLength) LPTSTR pszAllocatedBuffer,
    _In_ size_t nBufferLength) const
{
    bool bResult = false;
    if (nullptr != pszAllocatedBuffer)
    {
        *pszAllocatedBuffer = TEXT('\x00');

        LPTSTR pszResourceText;
        size_t nTextLength =
            ::LoadString(m_hModuleWithResource, nStringResourceId, reinterpret_cast<LPTSTR> (&pszResourceText), 0);
        if (nTextLength && (nTextLength < nBufferLength))
        {
            _tcsncpy_s(pszAllocatedBuffer, nBufferLength, pszResourceText, nTextLength);
            bResult = true;
        }
    }

    return bResult;
}


/**
 * Loads a string resource from the m_hModuleWithResource module, allocates a buffer w/ necessary length,
 * copies the string into a buffer, and appends a terminating NULL character.
 *
 * Parameters:
 * >nStringResourceId
 * Specifies the integer identifier of the string to be loaded.
 * >ppszResourceString
 * Pointer to pointer that stores address of the buffer that receives resource string. This buffer's content
 * is valid only when the method returns true value. In all other cases the loadString method sets the
 * '*ppszResourceString' to nullptr.
 * >pnResourceLength
 * Pointer to variable that receives loaded string length in TCHARs, not including NULL terminating character.
 * Caller should ignores this value when the loadString failed.
 * This parameter can be nullptr.
 *
 * Returns:
 * true value in case of success, false -- otherwise. In the first case caller should free the *ppszResult buffer
 * w/ `default_allocator`.
 */
_Success_(return) bool string_util::loadString(
    _In_ UINT nStringResourceId,
    _When_(0 != return, _Outptr_result_z_)
    _When_(0 == return, _Outptr_result_maybenull_z_) LPTSTR* ppszResourceString,
    _Out_opt_ size_t* pnResourceLength) const
{
    size_t nNotUsedResourceLength;
    if (!pnResourceLength)
    {
        pnResourceLength = &nNotUsedResourceLength;
    }
    *ppszResourceString = nullptr;
    bool bResult = false;
    LPTSTR pszResourceText;
    *pnResourceLength =
        ::LoadString(m_hModuleWithResource, nStringResourceId, reinterpret_cast<LPTSTR> (&pszResourceText), 0);
    if (*pnResourceLength)
    {
        STLADD default_allocator<TCHAR> allocator {};
        *ppszResourceString = allocator.allocate(*pnResourceLength + 1);
        if (*ppszResourceString)
        {
            _tcsncpy_s(*ppszResourceString, *pnResourceLength + 1, pszResourceText, *pnResourceLength);
            bResult = true;
        }
    }

    return bResult;
}


/**
 * Loads a string resource from the m_hModuleWithResource module. If any error has occurred result is empty pointer.
 *
 * Parameters:
 * >nStringResourceId
 * Specifies the integer identifier of the string to be loaded.
 *
 * Returns:
 * Unique pointer to a STLADD string_type object instance. Can be empty pointer.
 */
STLADD string_unique_ptr_t string_util::loadString(_In_ UINT nStringResourceId) const
{
    LPCTSTR pszText;
    size_t nLength;
    if (loadReadOnlyString(nStringResourceId, &pszText, &nLength))
    {
        return std::make_unique<STLADD string_type>(pszText, nLength);
    }
    else
    {
        return nullptr;
    }
}


/**
 * Loads a string resource from the m_hModuleWithResource module and returns read-only pointer to the resource itself.
 *
 * Parameters:
 * >nStringResourceId
 * Specifies the integer identifier of the string to be loaded.
 * >ppszResourceString
 * Receives a _read-only_ pointer to the resource itself. This pointer is valid only when the method returns true value.
 * In all other cases the loadReadOnlyString method sets the '*ppszResourceString' to nullptr.
 * A caller must not change content of the *ppszResourceString string 'cause this is DIRECT pointer to the resource
 * itself. Of course, this ain't NULL terminated string.
 * >pnResourceLength
 * Pointer to variable that receives resource string length in TCHARs. Caller should ignore this value when the
 * 'loadReadOnlyString' failed.
 *
 * Returns:
 * true value in case of success, false -- otherwise.
 * The ppszResourceString contains read-only pointer to the resource itself; caller must not try to free this buffer.
 */
_Success_(return) bool string_util::loadReadOnlyString(
    _In_ UINT nStringResourceId,
    _Outptr_result_z_ LPCTSTR* ppszResourceString,
    _Out_ size_t* pnResourceLength) const
{
    bool bResult;
    LPTSTR pszPointer;
    *pnResourceLength =
        ::LoadString(m_hModuleWithResource, nStringResourceId, reinterpret_cast<LPTSTR> (&pszPointer), 0);
    if (*pnResourceLength)
    {
        *ppszResourceString = pszPointer;
        bResult = true;
    }
    else
    {
        *ppszResourceString = nullptr;
        bResult = false;
    }
    return bResult;
}


/**
 * Wrapping method for the convenience of use the string_util class.
 * When one of the string_util methods allocates some memory resource and returns a pointer to that resource,
 * a caller has ability to free the memory w/o resorting to the help of `STLADD default_allocator`.
 * For example:
 *     string_util::const_ptr_t pStringUtil = string_util::getInstance();
 *     LPTSTR pszDateTime = pStringUtil->convertSystemTimeToString(...);
 *     if (pszDateTime) { . . .
 * When you don't need the pszDateTime anymore you can release it w/:
 * (a)
 *    STLADD default_allocator<TCHAR> allocator {};
 *    allocator.deallocate(pszResourceString);
 * or
 * (b)
 *    pStringUtil->freeString(pszDateTime);
 *
 * Parameters:
 * >pszResourceString
 * String buffer, that was allocated by a string_util's member function.
 *
 * Retuns:
 * N/A.
 */
void string_util::freeString(_In_ __drv_freesMem(Mem) _Frees_ptr_ LPTSTR pszResourceString) const
{
    STLADD default_allocator<TCHAR> allocator {};
    allocator.deallocate(pszResourceString);
}


/**
 * Loads format string from the application's resource, checks it and makes formatted result string.
 * Format string from resource should have only one "%s" field and this field only.
 *
 * Parameters:
 * >nResultFormatId
 * Resource identifier of string, that holds format-control string. This member checks it after loading from
 * resource and fails when that formatting string has fields other than "%s", or doesn't have "%s" at all, or
 * has more than one "%s" field.
 * >pszArgument
 * NULL-terminated string -- argument for the "%s" field in the formatting string.
 * >nArgumentLength
 * Length of the pszArgument NOT including NULL-terminating character.
 * >ppszResult
 * Address of variable, that receives address of formatted (result) string. This member allocates this buffer and
 * it's a caller's responsibility to free it. Of course, when the method fails, caller must ignore values of this and
 * the next parameters.
 * >pnResultLength
 * Pointer to variable that receives length of resultant formatted string. This value DOES NOT include NULL-terminating
 * character. Caller can pass NULL pointer here.
 *
 * Returns:
 * true value in case of success, false -- otherwise. In the first case caller should free the *ppszResult buffer
 * w/ memory_manager::free<LPTSTR> method.
 */
_Success_(return)
bool string_util::formatOneStringField(
    _In_ UINT nResultFormatId,
    _In_z_ LPCTSTR pszArgument,
    _In_ size_t nArgumentLength,
    _When_(0 != return, _Outptr_result_z_)
    _When_(0 == return, _Outptr_result_maybenull_z_) LPTSTR* ppszResult,
    _Out_opt_ size_t* pnResultLength) const
{
    size_t nNotUsedResultLength;
    if (!pnResultLength)
    {
        pnResultLength = &nNotUsedResultLength;
    }
    *ppszResult = nullptr;
    bool bResult = false;
    LPTSTR pszResultFormat;
    if (loadString(nResultFormatId, &pszResultFormat, pnResultLength))
    {
        STLADD default_allocator<TCHAR> allocator {};
        const STLADD regex_type regexOfFormat(TEXT("(?:[^%]*)(?:%s){1,1}(?:[^%]*)"));
        if (std::regex_match(pszResultFormat, pszResultFormat + (*pnResultLength), regexOfFormat))
        {
            *pnResultLength += (nArgumentLength - 1);
            *ppszResult = allocator.allocate(*pnResultLength);
            if (*ppszResult)
            {
                _stprintf_s(*ppszResult, *pnResultLength, pszResultFormat, pszArgument);
                --(*pnResultLength);
                bResult = true;
            }
        }
        allocator.deallocate(pszResultFormat);
    }
    return bResult;
}


/**
 * Converts SYSTEMTIME structure to string like "DATE_LONGDATE, TIME_NOSECONDS" or "DATE_SHORTDATE, TIME_NOSECONDS".
 * This member uses Vista's (tm) GetDateFormatEx/GetTimeFormatEx when the application is running under Windows(R) 6 or
 * higher, or GetDateFormat/GetTimeFormat functions for old systems.
 * When return value is not NULL, a caller have to free that memory w/ call to the freeString member.
 *
 * Parameters:
 * >pSystemTime
 * System time to be converted to string representation.
 * >bUseLongDateFormat
 * Pass true to use long date format (the LOCALE_SLONGDATE) in the result; false to use the short one (the
 * LOCALE_SSHORTDATE).
 * >bSearchForTwoDigitsFormat
 * Pass true to force the method to search a value for the LOCALE_SLONGDATE/LOCALE_SSHORTDATE format where year part has
 * only two digits. Pass false to use the first value of the LOCALE_SLONGDATE/LOCALE_SSHORTDATE format.
 * I don't believe one can find the LOCALE_SLONGDATE format w/ year part two digits long in _default_ locale settings.
 * >ppszString
 * Pointer to variable, that receives address of string buffer w/ result. This buffer is allocated by the member, and
 * when it completes its works successfully it is a caller responsibility to free that memory via the application's
 * memory manager or string_util::freeString method. But when the method fails a caller must ignore this and the next
 * out parameters.
 * >pnStringLength
 * Pointer to variable that receives string result length not including NULL-terminating character. Caller can pass
 * NULL pointer here.
 *
 * Returns:
 * true value in case of success, false -- otherwise.
 *
 * Remarks:
 * The method assumes that 'kernel32.dll' module is already loaded by the application.
 */
_Success_(return)
bool string_util::convertSystemTimeToString(
    _In_ const SYSTEMTIME* pSystemTime,
    _In_ const bool bUseLongDateFormat,
    _In_ const bool bSearchForTwoDigitsFormat,
    _When_(0 != return, _Outptr_result_z_)
    _When_(0 == return, _Outptr_result_maybenull_z_) LPTSTR* const ppszString,
    _Out_opt_ size_t* pnStringLength) const
{
    using enum_date_formats_ex_ex_func_t = BOOL (WINAPI *)(
        _In_ DATEFMT_ENUMPROCEXEX, _In_opt_ LPCWSTR, _In_ DWORD, _In_ LPARAM);
    using enum_date_formats_ex_func_t = BOOL (WINAPI *)(_In_ DATEFMT_ENUMPROCEXW, _In_ LCID, _In_ DWORD);

    using get_date_format_ex_func_t = int (WINAPI *)(_In_opt_ LPCWSTR,
                                                     _In_ DWORD,
                                                     _In_opt_ CONST SYSTEMTIME*,
                                                     _In_opt_ LPCWSTR,
                                                     _Out_writes_opt_(cchDate) LPWSTR,
                                                     _In_ int cchDate,
                                                     _In_opt_ LPCWSTR);
    using get_date_format_func_t = int (WINAPI *)(_In_ LCID,
                                                  _In_ DWORD,
                                                  _In_opt_ CONST SYSTEMTIME*,
                                                  _In_opt_ LPCWSTR,
                                                  _Out_writes_opt_(cchDate) LPWSTR,
                                                  _In_ int cchDate);
    using get_time_format_ex_func_t = int (WINAPI *)(_In_opt_ LPCWSTR,
                                                     _In_ DWORD,
                                                     _In_opt_ CONST SYSTEMTIME*,
                                                     _In_opt_ LPCWSTR,
                                                     _Out_writes_opt_(cchTime) LPWSTR,
                                                     _In_ int cchTime);
    using get_time_format_func_t = int (WINAPI *)(_In_ LCID,
                                                  _In_ DWORD,
                                                  _In_opt_ CONST SYSTEMTIME*,
                                                  _In_opt_ LPCWSTR,
                                                  _Out_writes_opt_(cchTime) LPWSTR,
                                                  _In_ int cchTime);

    get_locale_info_ex_func_t pGetLocaleInfoEx = nullptr;
    get_locale_info_func_t pGetLocaleInfo = nullptr;
    enum_date_formats_ex_ex_func_t pEnumDateFormatsExEx = nullptr;
    enum_date_formats_ex_func_t pEnumDateFormatsEx = nullptr;
    get_date_format_ex_func_t pGetDateFormatEx = nullptr;
    get_date_format_func_t pGetDateFormat = nullptr;
    get_time_format_ex_func_t pGetTimeFormatEx = nullptr;
    get_time_format_func_t pGetTimeFormat = nullptr;

    bool bWindowsVistaOrLater = false;
    HMODULE hKernel32 = nullptr;
    if (GetModuleHandleEx(0, TEXT("kernel32.dll"), &hKernel32) && hKernel32)
    {
        pGetLocaleInfoEx = getFunction<get_locale_info_ex_func_t>(nullptr, "GetLocaleInfoEx", hKernel32);
        pEnumDateFormatsExEx = getFunction<enum_date_formats_ex_ex_func_t>(nullptr, "EnumDateFormatsExEx", hKernel32);
        pGetDateFormatEx = getFunction<get_date_format_ex_func_t>(nullptr, "GetDateFormatEx", hKernel32);
        pGetTimeFormatEx = getFunction<get_time_format_ex_func_t>(nullptr, "GetTimeFormatEx", hKernel32);
        bWindowsVistaOrLater = (pGetLocaleInfoEx && pEnumDateFormatsExEx && pGetDateFormatEx && pGetTimeFormatEx);
        if (!bWindowsVistaOrLater)
        {
#if defined(_UNICODE)
            pGetLocaleInfo = getFunction<get_locale_info_func_t>(nullptr, "GetLocaleInfoW", hKernel32);
            pEnumDateFormatsEx = getFunction<enum_date_formats_ex_func_t>(nullptr, "EnumDateFormatsExW", hKernel32);
            pGetDateFormat = getFunction<get_date_format_func_t>(nullptr, "GetDateFormatW", hKernel32);
            pGetTimeFormat = getFunction<get_time_format_func_t>(nullptr, "GetTimeFormatW", hKernel32);
#else
            pGetLocaleInfo = getFunction<get_locale_info_func_t>(nullptr, "GetLocaleInfoA", hKernel32);
            pEnumDateFormatsEx = getFunction<enum_date_formats_ex_func_t>(nullptr, "EnumDateFormatsExA", hKernel32);
            pGetDateFormat = getFunction<get_date_format_func_t>(nullptr, "GetDateFormatA", hKernel32);
            pGetTimeFormat = getFunction<get_time_format_func_t>(nullptr, "GetTimeFormatA", hKernel32);
#endif
            // Well, the pGetLocaleInfo, pEnumDateFormatsEx, pGetDateFormat and pGetTimeFormat must be valid here.
            // I don't check it.
        }
    }

    size_t nNotUsedResultLength;
    if (!pnStringLength)
    {
        pnStringLength = &nNotUsedResultLength;
    }
    *pnStringLength = 0;
    bool bResult = false;
    if (bWindowsVistaOrLater || (pGetLocaleInfo && pEnumDateFormatsEx && pGetDateFormat && pGetTimeFormat))
    {
        LCTYPE nLocaleDateFormat = bUseLongDateFormat ? LOCALE_SLONGDATE : LOCALE_SSHORTDATE;
        int nFormatLength = (pGetLocaleInfoEx ?
                            (pGetLocaleInfoEx)(LOCALE_NAME_USER_DEFAULT, nLocaleDateFormat, nullptr, 0) :
                            (pGetLocaleInfo)(LOCALE_USER_DEFAULT, nLocaleDateFormat, nullptr, 0));
        if (nFormatLength)
        {
            STLADD default_allocator<TCHAR> allocator {};
            LPTSTR pszFormat = allocator.allocate(nFormatLength);
            if (nullptr != pszFormat)
            {
                if (pGetLocaleInfoEx)
                {
                    if (!(pGetLocaleInfoEx)(LOCALE_NAME_USER_DEFAULT, nLocaleDateFormat, pszFormat, nFormatLength))
                    {
                        allocator.deallocate(pszFormat);
                        pszFormat = nullptr;
                    }
                }
                else
                {
                    if (!(pGetLocaleInfo)(LOCALE_USER_DEFAULT, nLocaleDateFormat, pszFormat, nFormatLength))
                    {
                        allocator.deallocate(pszFormat);
                        pszFormat = nullptr;
                    }
                }

                DWORD nFlags = bUseLongDateFormat ? DATE_LONGDATE : DATE_SHORTDATE;
                if (bSearchForTwoDigitsFormat)
                {
                    if (nullptr != pszFormat)
                    {
                        // Check if the pszFormat formatting string has only two-digits year type.
                        const STLADD regex_type regexOfFormat(TEXT("(?:^yy[^y].*)|(?:.*[^y]yy[^y].*)|(?:.*[^y]yy$)"));
                        if (!std::regex_match(pszFormat, pszFormat + nFormatLength, regexOfFormat))
                        {
                            allocator.deallocate(pszFormat);
                            pszFormat = nullptr;
                        }
                    }

                    if (nullptr == pszFormat)
                    {
                        /*
                         * Search other, non-default LOCALE_SSHORTDATE formatting string.
                         * The m_pszEnumDateFormatsProcExData data memeber is used by the
                         * string_util::enumDateFormatsProcEx static method.
                         */
                        m_ppszEnumDateFormatsProcExData = &pszFormat;
                        BOOL bEnumResult =
                            (pEnumDateFormatsExEx ?
                            (pEnumDateFormatsExEx)(enumDateFormatsProcExEx,
                                                   LOCALE_NAME_USER_DEFAULT,
                                                   nFlags,
                                                   reinterpret_cast<LPARAM> (&pszFormat)) :
                            (pEnumDateFormatsEx)(enumDateFormatsProcEx, LOCALE_USER_DEFAULT, nFlags));
                        if (!bEnumResult && pszFormat)
                        {
                            allocator.deallocate(pszFormat);
                            pszFormat = nullptr;
                        }
                    }
                }

                // Use found or standard formatting string to format the pSystemTime.
                const DWORD nGetDateFlags = pszFormat ? 0 : nFlags;
                STLADD string_unique_ptr_t szListSeparator = getListSeparator();
                size_t nSplitterLength = szListSeparator->size();
                size_t nDateBufferSize;
                // Shut the C4701 down ("potentially uninitialized local variable 'nTimeBufferSize' used").
                size_t nTimeBufferSize = 0;
                // Calculate buffer size big enough to hold date, time and list separator.
                if (pGetDateFormatEx)
                {
                    nDateBufferSize = (pGetDateFormatEx)(LOCALE_NAME_USER_DEFAULT,
                                                         nGetDateFlags,
                                                         pSystemTime,
                                                         pszFormat,
                                                         nullptr,
                                                         0,
                                                         nullptr);
                    if (nDateBufferSize)
                    {
                        nTimeBufferSize = (pGetTimeFormatEx)(LOCALE_NAME_USER_DEFAULT,
                                                             TIME_NOSECONDS,
                                                             pSystemTime,
                                                             nullptr,
                                                             nullptr,
                                                             0);
                        if (nTimeBufferSize)
                        {
                            *pnStringLength = nDateBufferSize + nTimeBufferSize - 1 + nSplitterLength;
                        }
                        else
                        {
                            *pnStringLength = 0;
                        }
                    }
                    else
                    {
                        *pnStringLength = 0;
                    }
                }
                else
                {
                    // "Under-Vista" path.
                    nDateBufferSize = (pGetDateFormat)(LOCALE_USER_DEFAULT,
                                                       nGetDateFlags,
                                                       pSystemTime,
                                                       pszFormat,
                                                       nullptr,
                                                       0);
                    if (nDateBufferSize)
                    {
                        nTimeBufferSize = (pGetTimeFormat)(LOCALE_USER_DEFAULT,
                                                           TIME_NOSECONDS,
                                                           pSystemTime,
                                                           nullptr,
                                                           nullptr,
                                                           0);
                        if (nTimeBufferSize)
                        {
                            *pnStringLength = nDateBufferSize + nTimeBufferSize - 1 + nSplitterLength;
                        }
                        else
                        {
                            *pnStringLength = 0;
                        }
                    }
                    else
                    {
                        *pnStringLength = 0;
                    }
                }
                if (*pnStringLength)
                {
                    *ppszString = allocator.allocate(*pnStringLength);
                    if (*ppszString)
                    {
                        if (pGetDateFormatEx)
                        {
                            nDateBufferSize = (pGetDateFormatEx)(LOCALE_NAME_USER_DEFAULT,
                                                                 nGetDateFlags,
                                                                 pSystemTime,
                                                                 pszFormat,
                                                                 *ppszString,
                                                                 static_cast<int> (nDateBufferSize),
                                                                 nullptr);
                            if (nDateBufferSize)
                            {
                                _tcscat_s(*ppszString, *pnStringLength, szListSeparator->c_str());
                                nTimeBufferSize = (pGetTimeFormatEx)(
                                    LOCALE_NAME_USER_DEFAULT,
                                    TIME_NOSECONDS,
                                    pSystemTime,
                                    nullptr,
                                    (*ppszString) + nDateBufferSize + nSplitterLength - 1,
                                    static_cast<int> (nTimeBufferSize));
                            }
                        }
                        else
                        {
                            nDateBufferSize = (pGetDateFormat)(
                                LOCALE_USER_DEFAULT,
                                nGetDateFlags,
                                pSystemTime,
                                pszFormat,
                                *ppszString,
                                static_cast<int> (nDateBufferSize));
                            if (nDateBufferSize)
                            {
                                _tcscat_s(*ppszString, *pnStringLength, szListSeparator->c_str());
                                nTimeBufferSize = (pGetTimeFormat)(
                                    LOCALE_USER_DEFAULT,
                                    TIME_NOSECONDS,
                                    pSystemTime,
                                    nullptr,
                                    (*ppszString) + nDateBufferSize + nSplitterLength - 1,
                                    static_cast<int> (nTimeBufferSize));
                            }
                        }
                        if (nDateBufferSize && nTimeBufferSize)
                        {
                            // Exclude NULL terminating character.
                            --(*pnStringLength);
                            bResult = true;
                        }
                        else
                        {
                            // Error has occurred.
                            allocator.deallocate(*ppszString);
                            *ppszString = nullptr;
                            *pnStringLength = 0;
                        }
                    }
                }

                if (nullptr != pszFormat)
                {
                    allocator.deallocate(pszFormat);
                }
            }
        }
    }
    if (nullptr != hKernel32)
    {
        FreeLibrary(hKernel32);
    }

    return bResult;
}


/**
 * Converts SYSTEMTIME structure to string like "DATE_LONGDATE" or "DATE_SHORTDATE".
 * This member uses Vista's (tm) GetTimeFormatEx when the application is running under Windows(R) 6 or higher,
 * or GetTimeFormat function for old systems.
 *
 * Parameters:
 * >pSystemTime
 * System time to be converted to string representation.
 * >bUseLongDateFormat
 * Pass true to use long date format (the LOCALE_SLONGDATE) in the result; false to use the short one (the
 * LOCALE_SSHORTDATE).
 * >bSearchForTwoDigitsFormat
 * Pass true to force the method to search a value for the LOCALE_SLONGDATE/LOCALE_SSHORTDATE format where year part has
 * only two digits. Pass false to use the first value of the LOCALE_SLONGDATE/LOCALE_SSHORTDATE format.
 * I don't believe one can find the LOCALE_SLONGDATE format w/ year part two digits long in _default_ locale settings.
 * >ppszString
 * Pointer to variable, that receives address of string buffer w/ result. This buffer is allocated by the member, and
 * when it completes its works successfully it is a caller responsibility to free that memory via the application's
 * memory manager or string_util::freeString method. But when the method fails a caller must ignore this and the next
 * out parameters.
 * >pnStringLength
 * Pointer to variable that receives string result length not including NULL-terminating character. Caller can pass
 * NULL pointer here.
 *
 * Returns:
 * true value in case of success, false -- otherwise.
 *
 * Remarks:
 * The method assumes that 'kernel32.dll' module is already loaded by the application.
 */
_Success_(return)
bool string_util::convertSystemDateOnlyToString(
    _In_ const SYSTEMTIME* pSystemTime,
    _In_ const bool bUseLongDateFormat,
    _In_ const bool bSearchForTwoDigitsFormat,
    _When_(0 != return, _Outptr_result_z_)
    _When_(0 == return, _Outptr_result_maybenull_z_) LPTSTR* const ppszString,
    _Out_opt_ size_t* pnStringLength) const
{
    using enum_date_formats_ex_ex_func_t = BOOL (WINAPI *)(
        _In_ DATEFMT_ENUMPROCEXEX, _In_opt_ LPCWSTR, _In_ DWORD, _In_ LPARAM);
    using enum_date_formats_ex_func_t = BOOL (WINAPI *)(_In_ DATEFMT_ENUMPROCEXW, _In_ LCID, _In_ DWORD);

    using get_date_format_ex_func_t = int (WINAPI *)(_In_opt_ LPCWSTR,
                                                     _In_ DWORD,
                                                     _In_opt_ CONST SYSTEMTIME*,
                                                     _In_opt_ LPCWSTR,
                                                     _Out_writes_opt_(cchDate) LPWSTR,
                                                     _In_ int cchDate,
                                                     _In_opt_ LPCWSTR);
    using get_date_format_func_t = int (WINAPI *)(_In_ LCID,
                                                  _In_ DWORD,
                                                  _In_opt_ CONST SYSTEMTIME*,
                                                  _In_opt_ LPCWSTR,
                                                  _Out_writes_opt_(cchDate) LPWSTR,
                                                  _In_ int cchDate);

    get_locale_info_ex_func_t pGetLocaleInfoEx = nullptr;
    get_locale_info_func_t pGetLocaleInfo = nullptr;
    enum_date_formats_ex_ex_func_t pEnumDateFormatsExEx = nullptr;
    enum_date_formats_ex_func_t pEnumDateFormatsEx = nullptr;
    get_date_format_ex_func_t pGetDateFormatEx = nullptr;
    get_date_format_func_t pGetDateFormat = nullptr;

    bool bWindowsVistaOrLater = false;
    HMODULE hKernel32 = nullptr;
    if (GetModuleHandleEx(0, TEXT("kernel32.dll"), &hKernel32) && hKernel32)
    {
        pGetLocaleInfoEx = getFunction<get_locale_info_ex_func_t>(nullptr, "GetLocaleInfoEx", hKernel32);
        pEnumDateFormatsExEx = getFunction<enum_date_formats_ex_ex_func_t>(nullptr, "EnumDateFormatsExEx", hKernel32);
        pGetDateFormatEx = getFunction<get_date_format_ex_func_t>(nullptr, "GetDateFormatEx", hKernel32);
        bWindowsVistaOrLater = (pGetLocaleInfoEx && pEnumDateFormatsExEx && pGetDateFormatEx);
        if (!bWindowsVistaOrLater)
        {
#if defined(_UNICODE)
            pGetLocaleInfo = getFunction<get_locale_info_func_t>(nullptr, "GetLocaleInfoW", hKernel32);
            pEnumDateFormatsEx = getFunction<enum_date_formats_ex_func_t>(nullptr, "EnumDateFormatsExW", hKernel32);
            pGetDateFormat = getFunction<get_date_format_func_t>(nullptr, "GetDateFormatW", hKernel32);
#else
            pGetLocaleInfo = getFunction<get_locale_info_func_t>(nullptr, "GetLocaleInfoA", hKernel32);
            pEnumDateFormatsEx = getFunction<enum_date_formats_ex_func_t>(nullptr, "EnumDateFormatsExA", hKernel32);
            pGetDateFormat = getFunction<get_date_format_func_t>(nullptr, "GetDateFormatA", hKernel32);
#endif
            // Well, the pGetLocaleInfo, pEnumDateFormatsEx and pGetDateFormat must be valid here. I don't check it.
        }
    }

    size_t nNotUsedResultLength;
    if (!pnStringLength)
    {
        pnStringLength = &nNotUsedResultLength;
    }
    bool bResult = false;
    if (bWindowsVistaOrLater || (pGetLocaleInfo && pEnumDateFormatsEx && pGetDateFormat))
    {
        const LCTYPE nLocaleDateFormat = bUseLongDateFormat ? LOCALE_SLONGDATE : LOCALE_SSHORTDATE;
        int nFormatLength = (pGetLocaleInfoEx ?
                            (pGetLocaleInfoEx)(LOCALE_NAME_USER_DEFAULT, nLocaleDateFormat, nullptr, 0) :
                            (pGetLocaleInfo)(LOCALE_USER_DEFAULT, nLocaleDateFormat, nullptr, 0));
        if (nFormatLength)
        {
            STLADD default_allocator<TCHAR> allocator {};
            LPTSTR pszFormat = allocator.allocate(nFormatLength);
            if (nullptr != pszFormat)
            {
                if (pGetLocaleInfoEx)
                {
                    if (!(pGetLocaleInfoEx)(LOCALE_NAME_USER_DEFAULT, nLocaleDateFormat, pszFormat, nFormatLength))
                    {
                        allocator.deallocate(pszFormat);
                        pszFormat = nullptr;
                    }
                }
                else
                {
                    if (!(pGetLocaleInfo)(LOCALE_USER_DEFAULT, nLocaleDateFormat, pszFormat, nFormatLength))
                    {
                        allocator.deallocate(pszFormat);
                        pszFormat = nullptr;
                    }
                }

                const DWORD nFlags = bUseLongDateFormat ? DATE_LONGDATE : DATE_SHORTDATE;
                if (bSearchForTwoDigitsFormat)
                {
                    if (nullptr != pszFormat)
                    {
                        // Check if the pszFormat formatting string has only two-digits year type.
                        const STLADD regex_type regexOfFormat(TEXT("(?:^yy[^y].*)|(?:.*[^y]yy[^y].*)|(?:.*[^y]yy$)"));
                        if (!std::regex_match(pszFormat, pszFormat + nFormatLength, regexOfFormat))
                        {
                            allocator.deallocate(pszFormat);
                            pszFormat = nullptr;
                        }
                    }

                    if (nullptr == pszFormat)
                    {
                        /*
                         * Search other, non-default LOCALE_SSHORTDATE formatting string.
                         * The m_pszEnumDateFormatsProcExData data memeber is used by the
                         * string_util::enumDateFormatsProcEx static method.
                         */
                        m_ppszEnumDateFormatsProcExData = &pszFormat;
                        BOOL bEnumResult =
                            (pEnumDateFormatsExEx ?
                            (pEnumDateFormatsExEx)(enumDateFormatsProcExEx,
                                                   LOCALE_NAME_USER_DEFAULT,
                                                   nFlags,
                                                   reinterpret_cast<LPARAM> (&pszFormat)) :
                            (pEnumDateFormatsEx)(enumDateFormatsProcEx, LOCALE_USER_DEFAULT, nFlags));
                        if (!bEnumResult && pszFormat)
                        {
                            allocator.deallocate(pszFormat);
                            pszFormat = nullptr;
                        }
                    }
                }

                // Use found or standard formatting string to format the pSystemTime.
                const DWORD nGetDateFlags = (pszFormat ? 0 : nFlags);
                if (pGetDateFormatEx)
                {
                    *pnStringLength = (pGetDateFormatEx)(LOCALE_NAME_USER_DEFAULT,
                                                         nGetDateFlags,
                                                         pSystemTime,
                                                         pszFormat,
                                                         nullptr,
                                                         0,
                                                         nullptr);
                }
                else
                {
                    *pnStringLength = (pGetDateFormat)(LOCALE_USER_DEFAULT,
                                                       nGetDateFlags,
                                                       pSystemTime,
                                                       pszFormat,
                                                       nullptr,
                                                       0);
                }
                if (*pnStringLength)
                {
                    *ppszString = allocator.allocate(*pnStringLength);
                    if (*ppszString)
                    {
                        if (pGetDateFormatEx)
                        {
                            *pnStringLength = (pGetDateFormatEx)(LOCALE_NAME_USER_DEFAULT,
                                                                 nGetDateFlags,
                                                                 pSystemTime,
                                                                 pszFormat,
                                                                 *ppszString,
                                                                 static_cast<int> ((*pnStringLength)),
                                                                 nullptr);
                        }
                        else
                        {
                            *pnStringLength = (pGetDateFormat)(LOCALE_USER_DEFAULT,
                                                               nGetDateFlags,
                                                               pSystemTime,
                                                               pszFormat,
                                                               *ppszString,
                                                               static_cast<int> ((*pnStringLength)));
                        }
                        if (*pnStringLength)
                        {
                            // Exclude NULL terminating character.
                            --(*pnStringLength);
                            bResult = true;
                        }
                        else
                        {
                            // Error has occurred.
                            allocator.deallocate(*ppszString);
                            *ppszString = nullptr;
                        }
                    }
                }

                if (nullptr != pszFormat)
                {
                    allocator.deallocate(pszFormat);
                }
            }
        }
    }
    if (nullptr != hKernel32)
    {
        FreeLibrary(hKernel32);
    }

    return bResult;
}


/**
 * Converts SYSTEMTIME structure to string like "TIME_NOSECONDS".
 * This member uses Vista's (tm) GetTimeFormatEx when the application is running under Windows(R) 6 or higher,
 * or GetTimeFormat function for old systems.
 *
 * Parameters:
 * >pSystemTime
 * System time to be converted to string representation.
 * >ppszString
 * Pointer to variable, that receives address of string buffer w/ result. This buffer is allocated by the member, and
 * when it completes its works successfully it is a caller responsibility to free that memory via the application's
 * memory manager or string_util::freeString method. But when the method fails a caller must ignore this and the next
 * out parameters.
 * >pnStringLength
 * Pointer to variable that receives string result length not including NULL-terminating character.
 *
 * Returns:
 * true value in case of success, false -- otherwise.
 */
_Success_(return)
bool string_util::convertSystemTimeOnlyToString(
    _In_ const SYSTEMTIME* pSystemTime,
    _When_(0 != return, _Outptr_result_z_)
    _When_(0 == return, _Outptr_result_maybenull_z_) LPTSTR* const ppszString,
    _Out_ size_t* pnStringLength) const
{
    using get_time_format_ex_func_t = int (WINAPI *)(_In_opt_ LPCWSTR,
                                                     _In_ DWORD,
                                                     _In_opt_ CONST SYSTEMTIME*,
                                                     _In_opt_ LPCWSTR,
                                                     _Out_writes_opt_(cchTime) LPWSTR,
                                                     _In_ int cchTime);
    using get_time_format_func_t = int (WINAPI *)(_In_ LCID,
                                                  _In_ DWORD,
                                                  _In_opt_ CONST SYSTEMTIME*,
                                                  _In_opt_ LPCWSTR,
                                                  _Out_writes_opt_(cchTime) LPWSTR,
                                                  _In_ int cchTime);

    bool bResult = false;
    get_time_format_func_t pGetTimeFormat = nullptr;
    HMODULE hKernel32 = nullptr;
    get_time_format_ex_func_t pGetTimeFormatEx =
        getFunction<get_time_format_ex_func_t>(TEXT("kernel32.dll"), "GetTimeFormatEx", hKernel32);
    if (!pGetTimeFormatEx)
    {
#if defined(_UNICODE)
        pGetTimeFormat = getFunction<get_time_format_func_t>(TEXT("kernel32.dll"), "GetTimeFormatW", hKernel32);
#else
        pGetTimeFormat = getFunction<get_time_format_func_t>(TEXT("kernel32.dll"), "GetTimeFormatA", hKernel32);
#endif
    }

    *pnStringLength = 0;
    STLADD default_allocator<TCHAR> allocator {};
    if (pGetTimeFormatEx)
    {
        *pnStringLength =
            (pGetTimeFormatEx)(LOCALE_NAME_USER_DEFAULT, TIME_NOSECONDS, pSystemTime, nullptr, nullptr, 0);
    }
    else if (pGetTimeFormat)
    {
        // For system older than Vista(tm).
        *pnStringLength = (pGetTimeFormat)(LOCALE_USER_DEFAULT, TIME_NOSECONDS, pSystemTime, nullptr, nullptr, 0);
    }
    if (*pnStringLength)
    {
        *ppszString = allocator.allocate(*pnStringLength);
        if (*ppszString)
        {
            if (pGetTimeFormatEx)
            {
                *pnStringLength = (pGetTimeFormatEx)(LOCALE_NAME_USER_DEFAULT,
                                                     TIME_NOSECONDS,
                                                     pSystemTime,
                                                     nullptr,
                                                     *ppszString,
                                                     static_cast<int> ((*pnStringLength)));
            }
            else if (pGetTimeFormat)
            {
                *pnStringLength = (pGetTimeFormat)(LOCALE_USER_DEFAULT,
                                                   TIME_NOSECONDS,
                                                   pSystemTime,
                                                   nullptr,
                                                   *ppszString,
                                                   static_cast<int> ((*pnStringLength)));
            }

            if (*pnStringLength)
            {
                // Exclude NULL terminating character.
                --(*pnStringLength);
                bResult = true;
            }
            else
            {
                // Error has occurred.
                allocator.deallocate(*ppszString);
                *ppszString = nullptr;
            }
        }
    }
    if (nullptr != hKernel32)
    {
        FreeLibrary(hKernel32);
    }

    return bResult;
}


/**
 * Formats the specified number as a number string customized for the current user locale.
 *
 * Parameters:
 * >value
 * A number to be converted.
 *
 * Returns:
 * Unique pointer to result string. Can be empty pointer.
 */
template <typename T>
_Check_return_ STLADD string_unique_ptr_t string_util::formatNumber(_In_ const T value) const
{
    using get_number_format_ex_func_t = int (WINAPI *)(_In_opt_ LPCWSTR,
                                                       _In_ DWORD,
                                                       _In_ LPCWSTR,
                                                       _In_opt_ CONST NUMBERFMTW*,
                                                       _Out_writes_opt_(cchNumber) LPWSTR,
                                                       _In_ int cchNumber);
    using get_number_format_func_t = int (WINAPI *)(_In_ LCID,
                                                    _In_ DWORD,
                                                    _In_ LPCTSTR,
                                                    _In_opt_ CONST NUMBERFMT*,
                                                    _Out_writes_opt_(cchNumber) LPTSTR,
                                                    _In_ int cchNumber);

    STLADD string_unique_ptr_t szResult {};
    size_t nNumberLength = 32;
    STLADD default_allocator<TCHAR> allocator {};
    STLADD t_char_unique_ptr_t szNumber {allocator.allocate(nNumberLength)};
    nNumberLength = _stprintf_s(szNumber, nNumberLength, printf_value_trait<T>::get(), value);

    /*
     * Call the GetNumberFormatEx or GetNumberFormat function.
     */
    get_number_format_ex_func_t pGetNumberFormatEx = nullptr;
    get_number_format_func_t pGetNumberFormat = nullptr;
    get_locale_info_ex_func_t pGetLocaleInfoEx = nullptr;
    get_locale_info_func_t pGetLocaleInfo = nullptr;

    HMODULE hKernel32 = nullptr;
    if (GetModuleHandleEx(0, TEXT("kernel32.dll"), &hKernel32) && hKernel32)
    {
        pGetNumberFormatEx = getFunction<get_number_format_ex_func_t>(nullptr, "GetNumberFormatEx", hKernel32);
        pGetLocaleInfoEx = getFunction<get_locale_info_ex_func_t>(nullptr, "GetLocaleInfoEx", hKernel32);
        if (!(pGetNumberFormatEx && pGetLocaleInfoEx))
        {
#if defined(_UNICODE)
            pGetNumberFormat = getFunction<get_number_format_func_t>(nullptr, "GetNumberFormatW", hKernel32);
            pGetLocaleInfo = getFunction<get_locale_info_func_t>(nullptr, "GetLocaleInfoW", hKernel32);
#else
            pGetNumberFormat = getFunction<get_number_format_func_t>(nullptr, "GetNumberFormatA", hKernel32);
            pGetLocaleInfo = getFunction<get_locale_info_func_t>(nullptr, "GetLocaleInfoA", hKernel32);
#endif
            // Well, the pGetNumberFormat and pGetLocaleInfo must be valid here. I don't check it.
        }
    }

    if ((pGetNumberFormatEx && pGetLocaleInfoEx) || (pGetNumberFormat && pGetLocaleInfo))
    {
        NUMBERFMT numberFormat;
        /*
         * Fill the numberFormat structure.
         * Set number of fractional digits.
         */
#pragma warning(disable: 4127)
        if (std::is_floating_point<T>::value)
        {
            if (pGetLocaleInfoEx)
            {
                if (!(pGetLocaleInfoEx)(LOCALE_NAME_USER_DEFAULT,
                                        LOCALE_IDIGITS | LOCALE_RETURN_NUMBER,
                                        reinterpret_cast<LPTSTR> (&(numberFormat.NumDigits)),
                                        sizeof(numberFormat.NumDigits) / sizeof(TCHAR)))
                {
                    numberFormat.NumDigits = 2;
                }
            }
            else
            {
                if (!(pGetLocaleInfo)(LOCALE_USER_DEFAULT,
                                      LOCALE_IDIGITS | LOCALE_RETURN_NUMBER,
                                      reinterpret_cast<LPTSTR> (&(numberFormat.NumDigits)),
                                      sizeof(numberFormat.NumDigits) / sizeof(TCHAR)))
                {
                    numberFormat.NumDigits = 2;
                }
            }
        }
        else
        {
            numberFormat.NumDigits = 0;
        }
#pragma warning(default: 4127)
        // Leading zeros.
        if (pGetLocaleInfoEx)
        {
            if (!(pGetLocaleInfoEx)(LOCALE_NAME_USER_DEFAULT,
                                    LOCALE_ILZERO | LOCALE_RETURN_NUMBER,
                                    reinterpret_cast<LPTSTR> (&(numberFormat.LeadingZero)),
                                    sizeof(numberFormat.LeadingZero) / sizeof(TCHAR)))
            {
                numberFormat.LeadingZero = 1;
            }
        }
        else
        {
            if (!(pGetLocaleInfo)(LOCALE_USER_DEFAULT,
                                  LOCALE_ILZERO | LOCALE_RETURN_NUMBER,
                                  reinterpret_cast<LPTSTR> (&(numberFormat.LeadingZero)),
                                  sizeof(numberFormat.LeadingZero) / sizeof(TCHAR)))
            {
                numberFormat.LeadingZero = 1;
            }
        }
        // Set default (hard-coded) value to the numberFormat.Grouping.
        numberFormat.Grouping = 3;
        LPTSTR pszInfo;
        int nInfoLength =
            pGetLocaleInfoEx ?
            (pGetLocaleInfoEx)(LOCALE_NAME_USER_DEFAULT, LOCALE_SGROUPING, nullptr, 0) :
            (pGetLocaleInfo)(LOCALE_USER_DEFAULT, LOCALE_SGROUPING, nullptr, 0);
        if (nInfoLength)
        {
            pszInfo = allocator.allocate((static_cast<size_t> (nInfoLength)) << 1);
            STLADD t_char_unique_ptr_t guard {pszInfo};
            nInfoLength =
                pGetLocaleInfoEx ?
                (pGetLocaleInfoEx)(LOCALE_NAME_USER_DEFAULT, LOCALE_SGROUPING, pszInfo, nInfoLength) :
                (pGetLocaleInfo)(LOCALE_USER_DEFAULT, LOCALE_SGROUPING, pszInfo, nInfoLength);
            if (nInfoLength)
            {
                LPTSTR pszSource = pszInfo;
                LPTSTR pszDest = pszInfo + nInfoLength;
                while (TEXT('\x00') != *pszSource)
                {
                    // Skip ';' and '0' symbols at the end.

                    if ((TEXT(';') != *pszSource) &&
                        ((TEXT('0') != *pszSource) || (TEXT('\x00') != *(pszSource + 1))))
                    {
                        *pszDest = *pszSource;
                        ++pszDest;
                    }
                    ++pszSource;
                }
                *pszDest = TEXT('\x00');

                numberFormat.Grouping = _tcstoul(pszInfo + nInfoLength, nullptr, 10);
            }
        }
        numberFormat.lpDecimalSep = nullptr;
        nInfoLength =
            pGetLocaleInfoEx ?
            (pGetLocaleInfoEx)(LOCALE_NAME_USER_DEFAULT, LOCALE_SDECIMAL, nullptr, 0) :
            (pGetLocaleInfo)(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, nullptr, 0);
        if (nInfoLength)
        {
            pszInfo = allocator.allocate(nInfoLength);
            if (pGetLocaleInfoEx)
            {
                if ((pGetLocaleInfoEx)(LOCALE_NAME_USER_DEFAULT, LOCALE_SDECIMAL, pszInfo, nInfoLength))
                {
                    numberFormat.lpDecimalSep = pszInfo;
                }
                else
                {
                    allocator.deallocate(pszInfo);
                }
            }
            else
            {
                if ((pGetLocaleInfo)(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, pszInfo, nInfoLength))
                {
                    numberFormat.lpDecimalSep = pszInfo;
                }
                else
                {
                    allocator.deallocate(pszInfo);
                }
            }
        }
        numberFormat.lpThousandSep = nullptr;
        nInfoLength =
            pGetLocaleInfoEx ?
            (pGetLocaleInfoEx)(LOCALE_NAME_USER_DEFAULT, LOCALE_STHOUSAND, nullptr, 0) :
            (pGetLocaleInfo)(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, nullptr, 0);
        if (nInfoLength)
        {
            pszInfo = allocator.allocate(nInfoLength);
            if (pGetLocaleInfoEx)
            {
                if ((pGetLocaleInfoEx)(LOCALE_NAME_USER_DEFAULT, LOCALE_STHOUSAND, pszInfo, nInfoLength))
                {
                    numberFormat.lpThousandSep = pszInfo;
                }
                else
                {
                    allocator.deallocate(pszInfo);
                }
            }
            else
            {
                if ((pGetLocaleInfo)(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, pszInfo, nInfoLength))
                {
                    numberFormat.lpThousandSep = pszInfo;
                }
                else
                {
                    allocator.deallocate(pszInfo);
                }
            }
        }
        if (pGetLocaleInfoEx)
        {
            if (!(pGetLocaleInfoEx)(LOCALE_NAME_USER_DEFAULT,
                                    LOCALE_INEGNUMBER | LOCALE_RETURN_NUMBER,
                                    reinterpret_cast<LPTSTR> (&(numberFormat.NegativeOrder)),
                                    sizeof(numberFormat.NegativeOrder) / sizeof(TCHAR)))
            {
                numberFormat.NegativeOrder = 1;
            }
        }
        else
        {
            if (!(pGetLocaleInfo)(LOCALE_USER_DEFAULT,
                                    LOCALE_INEGNUMBER | LOCALE_RETURN_NUMBER,
                                    reinterpret_cast<LPTSTR> (&(numberFormat.NegativeOrder)),
                                    sizeof(numberFormat.NegativeOrder) / sizeof(TCHAR)))
            {
                numberFormat.NegativeOrder = 1;
            }
        }

        /*
         * Format the number.
         */
        int nFormattedNumberLength =
            pGetNumberFormatEx ?
            (pGetNumberFormatEx)(LOCALE_NAME_USER_DEFAULT, 0, szNumber, &numberFormat, nullptr, 0) :
            (pGetNumberFormat)(LOCALE_USER_DEFAULT, 0, szNumber, &numberFormat, nullptr, 0);
        if (nFormattedNumberLength)
        {
            LPTSTR pszFormattedNumber = allocator.allocate(nFormattedNumberLength);
            STLADD t_char_unique_ptr_t guard {pszFormattedNumber};
            if (pGetNumberFormatEx)
            {
                nFormattedNumberLength = (pGetNumberFormatEx)(
                    LOCALE_NAME_USER_DEFAULT, 0, szNumber, &numberFormat, pszFormattedNumber, nFormattedNumberLength);
            }
            else
            {
                nFormattedNumberLength = (pGetNumberFormat)(
                    LOCALE_USER_DEFAULT, 0, szNumber, &numberFormat, pszFormattedNumber, nFormattedNumberLength);
            }
            szResult = std::make_unique<STLADD string_type>(pszFormattedNumber, nFormattedNumberLength - 1);
        }

        if (numberFormat.lpThousandSep)
        {
            allocator.deallocate(numberFormat.lpThousandSep);
        }
        if (numberFormat.lpDecimalSep)
        {
            allocator.deallocate(numberFormat.lpDecimalSep);
        }
    }
    if (nullptr != hKernel32)
    {
        FreeLibrary(hKernel32);
    }

    if (!szResult)
    {
        szResult = std::make_unique<STLADD string_type>(szNumber, nNumberLength);
    }
    return szResult;
}


/**
 * A callback function for the EnumDateFormatsExEx function @ string_util::convertSystemDateOnlyToString method.
 *
 * Parameters:
 * >pszDateFormatString
 * Pointer to a buffer containing a null-terminated date format string. This string is a short date format.
 * >nCalendarID
 * Calendar identifier associated with the specified date format string.
 * >nParam
 * Pointer to a variable that should receive a pointer to application allocated memory.
 *
 * Returns:
 * TRUE to continue enumeration or FALSE otherwise.
 */
BOOL CALLBACK string_util::enumDateFormatsProcExEx(_In_z_ LPWSTR pszDateFormatString,
                                                   _In_ CALID nCalendarID,
                                                   _In_ LPARAM nParam)
{
    UNREFERENCED_PARAMETER(nCalendarID);
    BOOL bResult = TRUE;
    const STLADD regex_type regexOfFormat(TEXT("(?:^yy[^y].*)|(?:.*[^y]yy[^y].*)|(?:.*[^y]yy$)"));
    if (std::regex_match(pszDateFormatString, regexOfFormat))
    {
        size_t nFormatLength;
        if (SUCCEEDED(StringCchLength(pszDateFormatString, LOCALE_SSHORTDATE_MAX_LENGTH, &nFormatLength)))
        {
            STLADD default_allocator<TCHAR> allocator {};
            LPTSTR* ppszFormat = reinterpret_cast<LPTSTR*> (nParam);
            *ppszFormat = allocator.allocate(nFormatLength + 1);
            if (SUCCEEDED(StringCchCopy(*ppszFormat, nFormatLength + 1, pszDateFormatString)))
            {
                bResult = FALSE;
            }
            else
            {
                allocator.deallocate(*ppszFormat);
                *ppszFormat = nullptr;
            }
        }
    }
    return bResult;
}


/**
 * A callback function for the EnumDateFormatsExEx function @ string_util::convertSystemDateOnlyToString method.
 *
 * Parameters:
 * >pszDateFormatString
 * Pointer to a buffer containing a null-terminated date format string. This string is a short date format.
 * >nCalendarID
 * Calendar identifier associated with the specified date format string.
 *
 * Returns:
 * TRUE to continue enumeration or FALSE otherwise.
 */
BOOL CALLBACK string_util::enumDateFormatsProcEx(_In_z_ LPTSTR pszDateFormatString, _In_ CALID nCalendarID)
{
    return enumDateFormatsProcExEx(pszDateFormatString,
                                   nCalendarID,
                                   reinterpret_cast<LPARAM> (string_util::m_ppszEnumDateFormatsProcExData));
}


/**
 * Gets character(s) used to separate list items, as it's defined by user's regional settings.
 * So this method extracts the LOCALE_SLIST string.
 *
 * Parameters:
 * None.
 *
 * Returns:
 * Unique pointer to the string object. Result always has SPACE character at the end.
 */
STLADD string_unique_ptr_t string_util::getListSeparator() const
{
    get_locale_info_func_t pGetLocaleInfo = nullptr;
    HMODULE hKernel32 = nullptr;
    get_locale_info_ex_func_t pGetLocaleInfoEx =
        getFunction<get_locale_info_ex_func_t>(TEXT("kernel32.dll"), "GetLocaleInfoEx", hKernel32);
    if (!pGetLocaleInfoEx)
    {
#if defined(_UNICODE)
        pGetLocaleInfo = getFunction<get_locale_info_func_t>(nullptr, "GetLocaleInfoW", hKernel32);
#else
        pGetLocaleInfo = getFunction<get_locale_info_func_t>(nullptr, "GetLocaleInfoA", hKernel32);
#endif
        // Well, the pGetLocaleInfo must be valid here. I don't check it.
    }

    STLADD string_unique_ptr_t szResult {};

    if (pGetLocaleInfoEx || pGetLocaleInfo)
    {
        size_t nListSeparatorLength;
        if (pGetLocaleInfoEx)
        {
            nListSeparatorLength = (pGetLocaleInfoEx)(LOCALE_NAME_USER_DEFAULT, LOCALE_SLIST, nullptr, 0);
        }
        else
        {
            nListSeparatorLength = (pGetLocaleInfo)(LOCALE_USER_DEFAULT, LOCALE_SLIST, nullptr, 0);
        }

        if (nListSeparatorLength)
        {
            // Allocate memory to store list separator string only.
            STLADD default_allocator<TCHAR> allocator {};
            LPTSTR pszResult = allocator.allocate(nListSeparatorLength);
            STLADD t_char_unique_ptr_t guard {pszResult};
            if (pGetLocaleInfoEx)
            {
                nListSeparatorLength = (pGetLocaleInfoEx)(
                    LOCALE_NAME_USER_DEFAULT, LOCALE_SLIST, pszResult, static_cast<int> (nListSeparatorLength));
            }
            else
            {
                nListSeparatorLength = (pGetLocaleInfo)(
                    LOCALE_USER_DEFAULT, LOCALE_SLIST, pszResult, static_cast<int> (nListSeparatorLength));
            }
            if (nListSeparatorLength)
            {
                if (TEXT('\x20') != *(pszResult + nListSeparatorLength - 2))
                {
                    // Replace NULL termintaing character w/ 'space' character.
                    *(pszResult + nListSeparatorLength - 1) = TEXT('\x20');
                    szResult = std::make_unique<STLADD string_type>(pszResult, pszResult + nListSeparatorLength);
                }
                else
                {
                    szResult = std::make_unique<STLADD string_type>(pszResult, pszResult + nListSeparatorLength - 1);
                }
            }
        }
    }
    if (nullptr != hKernel32)
    {
        FreeLibrary(hKernel32);
    }

    if (!szResult)
    {
        szResult = std::make_unique<STLADD string_type>(DEFAULT_LIST_SEPARATOR, _countof(DEFAULT_LIST_SEPARATOR) - 1);
    }

    return szResult;
}


/**
 * Gets locale name for the current user (LOCALE_NAME_USER_DEFAULT).
 *
 * Parameters:
 * None.
 *
 * Returns:
 * The current user locale name.
 */
_Check_return_ STLADD string_unique_ptr_t string_util::getCurrentUserLocale() const
{
    get_locale_info_func_t pGetLocaleInfo = nullptr;
    HMODULE hKernel32 = nullptr;
    get_locale_info_ex_func_t pGetLocaleInfoEx =
        getFunction<get_locale_info_ex_func_t>(TEXT("kernel32.dll"), "GetLocaleInfoEx", hKernel32);
    if (!pGetLocaleInfoEx)
    {
#if defined(_UNICODE)
        pGetLocaleInfo = getFunction<get_locale_info_func_t>(nullptr, "GetLocaleInfoW", hKernel32);
#else
        pGetLocaleInfo = getFunction<get_locale_info_func_t>(nullptr, "GetLocaleInfoA", hKernel32);
#endif
        // Well, the pGetLocaleInfo must be valid here. I don't check it.
    }

    STLADD string_unique_ptr_t szResult {};

    if (pGetLocaleInfoEx || pGetLocaleInfo)
    {
        size_t nLength;
        if (pGetLocaleInfoEx)
        {
            nLength = (pGetLocaleInfoEx)(LOCALE_NAME_USER_DEFAULT, LOCALE_SNAME, nullptr, 0);
        }
        else
        {
            nLength = (pGetLocaleInfo)(LOCALE_USER_DEFAULT, LOCALE_SNAME, nullptr, 0);
        }

        if (nLength)
        {
            // Allocate memory to store list separator string only.
            STLADD default_allocator<TCHAR> allocator {};
            LPTSTR pszResult = allocator.allocate(nLength);
            STLADD t_char_unique_ptr_t guard {pszResult};
            if (pGetLocaleInfoEx)
            {
                nLength = (pGetLocaleInfoEx)(
                    LOCALE_NAME_USER_DEFAULT, LOCALE_SNAME, pszResult, static_cast<int> (nLength));
            }
            else
            {
                nLength = (pGetLocaleInfo)(
                    LOCALE_USER_DEFAULT, LOCALE_SNAME, pszResult, static_cast<int> (nLength));
            }
            if (nLength)
            {
                szResult = std::make_unique<STLADD string_type>(pszResult, pszResult + nLength - 1);
            }
        }
    }
    if (nullptr != hKernel32)
    {
        FreeLibrary(hKernel32);
    }

    return szResult;
}
#pragma endregion string_util class
