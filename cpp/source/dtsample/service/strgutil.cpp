#include "service\functype.h"
#include "service\miscutil.h"
#include "service\stladdon.h"
#include "service\strgutil.h"
#include <regex>
#include <tchar.h>
#include <strsafe.h>
#include <type_traits>

string_util::unique_ptr_t string_util::m_instance;
LPTSTR* string_util::m_enumDateFormatsProcExData;

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
 * Loads a string resource from the m_moduleWithResource module, copies the string into a buffer,
 * and appends a terminating NULL character.
 *
 * Parameters:
 * >stringResourceId
 * Specifies the integer identifier of the string to be loaded.
 * >allocatedBuffer
 * Pointer to the buffer to receive the string.
 * >bufferLength
 * Specifies the size of the buffer, in characters.
 *
 * Returns:
 * true value in case of success, false -- otherwise.
 */
_Success_(return) bool string_util::loadString(
    _In_ UINT stringResourceId,
    _Out_writes_opt_z_(bufferLength) LPTSTR allocatedBuffer,
    _In_ size_t bufferLength) const
{
    bool result = false;
    if (nullptr != allocatedBuffer)
    {
        *allocatedBuffer = TEXT('\x00');

        LPTSTR resourceText;
        const size_t textLength =
            ::LoadString(m_moduleWithResource, stringResourceId, reinterpret_cast<LPTSTR> (&resourceText), 0);
        if (textLength && (textLength < bufferLength))
        {
            _tcsncpy_s(allocatedBuffer, bufferLength, resourceText, textLength);
            result = true;
        }
    }

    return result;
}


/**
 * Loads a string resource from the m_moduleWithResource module, allocates a buffer w/ necessary length,
 * copies the string into a buffer, and appends a terminating NULL character.
 *
 * Parameters:
 * >stringResourceId
 * Specifies the integer identifier of the string to be loaded.
 * >resourceString
 * Pointer to pointer that stores address of the buffer that receives resource string. This buffer's content
 * is valid only when the method returns true value. In all other cases the loadString method sets the
 * '*resourceString' to nullptr.
 * >resourceLength
 * Pointer to variable that receives loaded string length in TCHARs, not including NULL terminating character.
 * Caller should ignores this value when the loadString failed.
 * This parameter can be nullptr.
 *
 * Returns:
 * true value in case of success, false -- otherwise. In the first case caller should free the `*resourceString` buffer
 * w/ `default_allocator` or `delete` operator.
 */
_Success_(return) bool string_util::loadString(
    _In_ UINT stringResourceId,
    _When_(0 != return, _Outptr_result_z_)
    _When_(0 == return, _Outptr_result_maybenull_z_) LPTSTR* resourceString,
    _Out_opt_ size_t* resourceLength) const
{
    size_t notUsedResourceLength;
    if (!resourceLength)
    {
        resourceLength = &notUsedResourceLength;
    }
    *resourceString = nullptr;
    bool result = false;
    LPTSTR resourceText;
    *resourceLength = ::LoadString(m_moduleWithResource, stringResourceId, reinterpret_cast<LPTSTR> (&resourceText), 0);
    if (*resourceLength)
    {
        *resourceString = new TCHAR[*resourceLength + 1];
        if (*resourceString)
        {
            _tcsncpy_s(*resourceString, *resourceLength + 1, resourceText, *resourceLength);
            result = true;
        }
    }

    return result;
}


/**
 * Loads a string resource from the m_moduleWithResource module. If any error has occurred result is empty pointer.
 *
 * Parameters:
 * >stringResourceId
 * Specifies the integer identifier of the string to be loaded.
 *
 * Returns:
 * Unique pointer to a STLADD string_type object instance. Can be empty pointer.
 */
STLADD string_unique_ptr_t string_util::loadString(_In_ UINT stringResourceId) const
{
    LPCTSTR text;
    size_t length;
    if (loadReadOnlyString(stringResourceId, &text, &length))
    {
        return std::make_unique<STLADD string_type>(text, length);
    }
    else
    {
        return nullptr;
    }
}


/**
 * Loads a string resource from the m_moduleWithResource module and returns read-only pointer to the resource itself.
 *
 * Parameters:
 * >stringResourceId
 * Specifies the integer identifier of the string to be loaded.
 * >resourceString
 * Receives a _read-only_ pointer to the resource itself. This pointer is valid only when the method returns true value.
 * In all other cases the loadReadOnlyString method sets the '*resourceString' to nullptr.
 * A caller must not change content of the *resourceString string 'cause this is DIRECT pointer to the resource
 * itself. Of course, this ain't NULL terminated string.
 * >resourceLength
 * Pointer to variable that receives resource string length in TCHARs. Caller should ignore this value when the
 * 'loadReadOnlyString' failed.
 *
 * Returns:
 * true value in case of success, false -- otherwise.
 * The resourceString contains read-only pointer to the resource itself; caller must not try to free this buffer.
 */
_Success_(return) bool string_util::loadReadOnlyString(
    _In_ UINT stringResourceId,
    _Outptr_result_z_ LPCTSTR* resourceString,
    _Out_ size_t* resourceLength) const
{
    LPTSTR p;
    *resourceLength = ::LoadString(m_moduleWithResource, stringResourceId, reinterpret_cast<LPTSTR> (&p), 0);
    if (*resourceLength)
    {
        *resourceString = p;
        return true;
    }
    else
    {
        *resourceString = nullptr;
        return false;
    }
}


/**
 * Loads format string from the application's resource, checks it and makes formatted result string.
 * Format string from resource should have only one "%s" field and this field only.
 *
 * Parameters:
 * >resultFormatId
 * Resource identifier of string, that holds format-control string. This member checks it after loading from
 * resource and fails when that formatting string has fields other than "%s", or doesn't have "%s" at all, or
 * has more than one "%s" field.
 * >argument
 * NULL-terminated string -- argument for the "%s" field in the formatting string.
 * >argumentLength
 * Length of the argument NOT including NULL-terminating character.
 * >string
 * Address of variable, that receives address of formatted (result) string. This member allocates this buffer and
 * it's a caller's responsibility to free it. Of course, when the method fails, caller must ignore values of this and
 * the next parameters.
 * >resultLength
 * Pointer to variable that receives length of resultant formatted string. This value DOES NOT include NULL-terminating
 * character. Caller can pass NULL pointer here.
 *
 * Returns:
 * true value in case of success, false -- otherwise. In the first case caller should free the *string buffer
 * w/ memory_manager::free<LPTSTR> method.
 */
_Success_(return)
bool string_util::formatOneStringField(
    _In_ UINT resultFormatId,
    _In_z_ LPCTSTR argument,
    _In_ size_t argumentLength,
    _When_(0 != return, _Outptr_result_z_)
    _When_(0 == return, _Outptr_result_maybenull_z_) LPTSTR* string,
    _Out_opt_ size_t* resultLength) const
{
    size_t notUsedResultLength;
    if (!resultLength)
    {
        resultLength = &notUsedResultLength;
    }
    *string = nullptr;
    bool result = false;
    LPTSTR resultFormat;
    if (loadString(resultFormatId, &resultFormat, resultLength))
    {
        STLADD t_char_unique_ptr_t guard {resultFormat};
        const STLADD regex_type regexOfFormat(TEXT("(?:[^%]*)(?:%s){1,1}(?:[^%]*)"));
        if (std::regex_match(resultFormat, resultFormat + (*resultLength), regexOfFormat))
        {
            *resultLength += argumentLength - 1;
            *string = new TCHAR[*resultLength];
            _stprintf_s(*string, *resultLength, resultFormat, argument);
            --(*resultLength);
            result = true;
        }
    }
    return result;
}


/**
 * Converts SYSTEMTIME structure to string like "DATE_LONGDATE, TIME_NOSECONDS" or "DATE_SHORTDATE, TIME_NOSECONDS".
 * This member uses Vista's (tm) GetDateFormatEx/GetTimeFormatEx when the application is running under Windows(R) 6 or
 * higher, or GetDateFormat/GetTimeFormat functions for old systems.
 * When return value is not NULL, a caller have to free that memory w/ call to the freeString member.
 *
 * Parameters:
 * >systemTime
 * System time to be converted to string representation.
 * >useLongDateFormat
 * Pass true to use long date format (the LOCALE_SLONGDATE) in the result; false to use the short one (the
 * LOCALE_SSHORTDATE).
 * >searchForTwoDigitsFormat
 * Pass true to force the method to search a value for the LOCALE_SLONGDATE/LOCALE_SSHORTDATE format where year part has
 * only two digits. Pass false to use the first value of the LOCALE_SLONGDATE/LOCALE_SSHORTDATE format.
 * I don't believe one can find the LOCALE_SLONGDATE format w/ year part two digits long in _default_ locale settings.
 * >string
 * Pointer to variable, that receives address of string buffer w/ result. This buffer is allocated by this method, and
 * when it completes its works successfully it is a caller responsibility to free that memory with `delete` operator.
 * But when the method fails a caller must ignore this and the next out parameters.
 * >stringLength
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
    _In_ const SYSTEMTIME* systemTime,
    _In_ const bool useLongDateFormat,
    _In_ const bool searchForTwoDigitsFormat,
    _When_(0 != return, _Outptr_result_z_)
    _When_(0 == return, _Outptr_result_maybenull_z_) LPTSTR* const string,
    _Out_opt_ size_t* stringLength) const
{
    using enum_date_formats_ex_ex_func_t = BOOL (WINAPI *)(
        _In_ DATEFMT_ENUMPROCEXEX, _In_opt_ LPCWSTR, _In_ DWORD, _In_ LPARAM);
    using enum_date_formats_ex_func_t = BOOL (WINAPI *)(_In_ DATEFMT_ENUMPROCEXW, _In_ LCID, _In_ DWORD);

    using get_date_format_ex_func_t = int (WINAPI *)(
        _In_opt_ LPCWSTR,
        _In_ DWORD,
        _In_opt_ CONST SYSTEMTIME*,
        _In_opt_ LPCWSTR,
        _Out_writes_opt_(cchDate) LPWSTR,
        _In_ int cchDate,
        _In_opt_ LPCWSTR);
    using get_date_format_func_t = int (WINAPI *)(
        _In_ LCID,
        _In_ DWORD,
        _In_opt_ CONST SYSTEMTIME*,
        _In_opt_ LPCWSTR,
        _Out_writes_opt_(cchDate) LPWSTR,
        _In_ int cchDate);
    using get_time_format_ex_func_t = int (WINAPI *)(
        _In_opt_ LPCWSTR,
        _In_ DWORD,
        _In_opt_ CONST SYSTEMTIME*,
        _In_opt_ LPCWSTR,
        _Out_writes_opt_(cchTime) LPWSTR,
        _In_ int cchTime);
    using get_time_format_func_t = int (WINAPI *)(
        _In_ LCID,
        _In_ DWORD,
        _In_opt_ CONST SYSTEMTIME*,
        _In_opt_ LPCWSTR,
        _Out_writes_opt_(cchTime) LPWSTR,
        _In_ int cchTime);

    get_locale_info_ex_func_t getLocaleInfoEx = nullptr;
    get_locale_info_func_t getLocaleInfo = nullptr;
    enum_date_formats_ex_ex_func_t enumDateFormatsExEx = nullptr;
    enum_date_formats_ex_func_t enumDateFormatsEx = nullptr;
    get_date_format_ex_func_t getDateFormatEx = nullptr;
    get_date_format_func_t getDateFormat = nullptr;
    get_time_format_ex_func_t getTimeFormatEx = nullptr;
    get_time_format_func_t getTimeFormat = nullptr;

    bool vistaOrLater = false;
    HMODULE kernel32 = nullptr;
    if (GetModuleHandleEx(0, TEXT("kernel32.dll"), &kernel32) && kernel32)
    {
        getLocaleInfoEx = getFunction<get_locale_info_ex_func_t>(nullptr, "GetLocaleInfoEx", kernel32);
        enumDateFormatsExEx = getFunction<enum_date_formats_ex_ex_func_t>(nullptr, "EnumDateFormatsExEx", kernel32);
        getDateFormatEx = getFunction<get_date_format_ex_func_t>(nullptr, "GetDateFormatEx", kernel32);
        getTimeFormatEx = getFunction<get_time_format_ex_func_t>(nullptr, "GetTimeFormatEx", kernel32);
        vistaOrLater = (getLocaleInfoEx && enumDateFormatsExEx && getDateFormatEx && getTimeFormatEx);
        if (!vistaOrLater)
        {
#if defined(_UNICODE)
            getLocaleInfo = getFunction<get_locale_info_func_t>(nullptr, "GetLocaleInfoW", kernel32);
            enumDateFormatsEx = getFunction<enum_date_formats_ex_func_t>(nullptr, "EnumDateFormatsExW", kernel32);
            getDateFormat = getFunction<get_date_format_func_t>(nullptr, "GetDateFormatW", kernel32);
            getTimeFormat = getFunction<get_time_format_func_t>(nullptr, "GetTimeFormatW", kernel32);
#else
            getLocaleInfo = getFunction<get_locale_info_func_t>(nullptr, "GetLocaleInfoA", kernel32);
            enumDateFormatsEx = getFunction<enum_date_formats_ex_func_t>(nullptr, "EnumDateFormatsExA", kernel32);
            getDateFormat = getFunction<get_date_format_func_t>(nullptr, "GetDateFormatA", kernel32);
            getTimeFormat = getFunction<get_time_format_func_t>(nullptr, "GetTimeFormatA", kernel32);
#endif
            // Well, the getLocaleInfo, enumDateFormatsEx, getDateFormat and getTimeFormat must be valid here.
            // I don't check it.
        }
    }

    size_t notUsedResultLength;
    if (!stringLength)
    {
        stringLength = &notUsedResultLength;
    }
    *stringLength = 0;
    bool result = false;
    if (vistaOrLater || (getLocaleInfo && enumDateFormatsEx && getDateFormat && getTimeFormat))
    {
        LCTYPE localeDateFormat = useLongDateFormat ? LOCALE_SLONGDATE : LOCALE_SSHORTDATE;
        int formatLength =
            (getLocaleInfoEx ?
            (getLocaleInfoEx)(LOCALE_NAME_USER_DEFAULT, localeDateFormat, nullptr, 0) :
            (getLocaleInfo)(LOCALE_USER_DEFAULT, localeDateFormat, nullptr, 0));
        if (formatLength)
        {
            STLADD t_char_unique_ptr_t formatGuard {new TCHAR[formatLength]};
            LPTSTR format {formatGuard.get()};
            if (getLocaleInfoEx)
            {
                if (!(getLocaleInfoEx)(LOCALE_NAME_USER_DEFAULT, localeDateFormat, format, formatLength))
                {
                    formatGuard.reset();
                    format = nullptr;
                }
            }
            else
            {
                if (!(getLocaleInfo)(LOCALE_USER_DEFAULT, localeDateFormat, format, formatLength))
                {
                    formatGuard.reset();
                    format = nullptr;
                }
            }

            DWORD flags = useLongDateFormat ? DATE_LONGDATE : DATE_SHORTDATE;
            if (searchForTwoDigitsFormat)
            {
                if (nullptr != format)
                {
                    // Check if the format formatting string has only two-digits year type.
                    const STLADD regex_type regexOfFormat {TEXT("(?:^yy[^y].*)|(?:.*[^y]yy[^y].*)|(?:.*[^y]yy$)")};
                    if (!std::regex_match(format, format + formatLength, regexOfFormat))
                    {
                        formatGuard.reset();
                        format = nullptr;
                    }
                }

                if (nullptr == format)
                {
                    /*
                     * Search other, non-default LOCALE_SSHORTDATE formatting string.
                     * The m_pszEnumDateFormatsProcExData data memeber is used by the
                     * `string_util::enumDateFormatsProcEx` static method.
                     */
                    m_enumDateFormatsProcExData = &format;
                    BOOL enumResult =
                        enumDateFormatsExEx ?
                        (enumDateFormatsExEx)(enumDateFormatsProcExEx,
                                              LOCALE_NAME_USER_DEFAULT,
                                              flags,
                                              reinterpret_cast<LPARAM> (&format)) :
                        (enumDateFormatsEx)(enumDateFormatsProcEx, LOCALE_USER_DEFAULT, flags);
                    if (!enumResult && format)
                    {
                        formatGuard.reset();
                        format = nullptr;
                    }
                }
            }

            // Use found or standard formatting string to format the systemTime.
            const DWORD getDateFlags = format ? 0 : flags;
            STLADD string_unique_ptr_t listSeparator = getListSeparator();
            size_t splitterLength = listSeparator->size();
            size_t dateBufferSize;
            // Shut the C4701 down ("potentially uninitialized local variable 'timeBufferSize' used").
            size_t timeBufferSize = 0;
            // Calculate buffer size big enough to hold date, time and list separator.
            if (getDateFormatEx)
            {
                dateBufferSize = (getDateFormatEx)(
                    LOCALE_NAME_USER_DEFAULT, getDateFlags, systemTime, format, nullptr, 0, nullptr);
                if (dateBufferSize)
                {
                    timeBufferSize = (getTimeFormatEx)(
                        LOCALE_NAME_USER_DEFAULT, TIME_NOSECONDS, systemTime, nullptr, nullptr, 0);
                    if (timeBufferSize)
                    {
                        *stringLength = dateBufferSize + timeBufferSize - 1 + splitterLength;
                    }
                    else
                    {
                        *stringLength = 0;
                    }
                }
                else
                {
                    *stringLength = 0;
                }
            }
            else
            {
                // "Pre-Vista" path.
                dateBufferSize = (getDateFormat)(LOCALE_USER_DEFAULT, getDateFlags, systemTime, format, nullptr, 0);
                if (dateBufferSize)
                {
                    timeBufferSize =
                        (getTimeFormat)(LOCALE_USER_DEFAULT, TIME_NOSECONDS, systemTime, nullptr, nullptr, 0);
                    if (timeBufferSize)
                    {
                        *stringLength = dateBufferSize + timeBufferSize - 1 + splitterLength;
                    }
                    else
                    {
                        *stringLength = 0;
                    }
                }
                else
                {
                    *stringLength = 0;
                }
            }
            if (*stringLength)
            {
                *string = new TCHAR[*stringLength];
                if (getDateFormatEx)
                {
                    dateBufferSize = (getDateFormatEx)(
                        LOCALE_NAME_USER_DEFAULT,
                        getDateFlags,
                        systemTime,
                        format,
                        *string,
                        static_cast<int> (dateBufferSize),
                        nullptr);
                    if (dateBufferSize)
                    {
                        _tcscat_s(*string, *stringLength, listSeparator->c_str());
                        timeBufferSize = (getTimeFormatEx)(
                            LOCALE_NAME_USER_DEFAULT,
                            TIME_NOSECONDS,
                            systemTime,
                            nullptr,
                            (*string) + dateBufferSize + splitterLength - 1,
                            static_cast<int> (timeBufferSize));
                    }
                }
                else
                {
                    dateBufferSize = (getDateFormat)(
                        LOCALE_USER_DEFAULT,
                        getDateFlags,
                        systemTime,
                        format,
                        *string,
                        static_cast<int> (dateBufferSize));
                    if (dateBufferSize)
                    {
                        _tcscat_s(*string, *stringLength, listSeparator->c_str());
                        timeBufferSize = (getTimeFormat)(
                            LOCALE_USER_DEFAULT,
                            TIME_NOSECONDS,
                            systemTime,
                            nullptr,
                            (*string) + dateBufferSize + splitterLength - 1,
                            static_cast<int> (timeBufferSize));
                    }
                }
                if (dateBufferSize && timeBufferSize)
                {
                    // Exclude NULL terminating character.
                    --(*stringLength);
                    result = true;
                }
                else
                {
                    // Error has occurred.
                    delete[] *string;
                    *string = nullptr;
                    *stringLength = 0;
                }
            }
        }
    }
    if (nullptr != kernel32)
    {
        FreeLibrary(kernel32);
    }

    return result;
}


/**
 * Converts SYSTEMTIME structure to string like "DATE_LONGDATE" or "DATE_SHORTDATE".
 * This member uses Vista's (tm) GetTimeFormatEx when the application is running under Windows(R) 6 or higher,
 * or GetTimeFormat function for old systems.
 *
 * Parameters:
 * >systemTime
 * System time to be converted to string representation.
 * >useLongDateFormat
 * Pass true to use long date format (the LOCALE_SLONGDATE) in the result; false to use the short one (the
 * LOCALE_SSHORTDATE).
 * >searchForTwoDigitsFormat
 * Pass true to force the method to search a value for the LOCALE_SLONGDATE/LOCALE_SSHORTDATE format where year part has
 * only two digits. Pass false to use the first value of the LOCALE_SLONGDATE/LOCALE_SSHORTDATE format.
 * I don't believe one can find the LOCALE_SLONGDATE format w/ year part two digits long in _default_ locale settings.
 * >string
 * Pointer to variable, that receives address of string buffer w/ result. This buffer is allocated by this method, and
 * when it completes its works successfully it is a caller responsibility to free that memory with `delete` opeartor.
 * But when the method fails a caller must ignore this and the next out parameters.
 * >stringLength
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
    _In_ const SYSTEMTIME* systemTime,
    _In_ const bool useLongDateFormat,
    _In_ const bool searchForTwoDigitsFormat,
    _When_(0 != return, _Outptr_result_z_)
    _When_(0 == return, _Outptr_result_maybenull_z_) LPTSTR* const string,
    _Out_opt_ size_t* stringLength) const
{
    using enum_date_formats_ex_ex_func_t = BOOL (WINAPI *)(
        _In_ DATEFMT_ENUMPROCEXEX, _In_opt_ LPCWSTR, _In_ DWORD, _In_ LPARAM);
    using enum_date_formats_ex_func_t = BOOL (WINAPI *)(_In_ DATEFMT_ENUMPROCEXW, _In_ LCID, _In_ DWORD);

    using get_date_format_ex_func_t = int (WINAPI *)(
        _In_opt_ LPCWSTR,
        _In_ DWORD,
        _In_opt_ CONST SYSTEMTIME*,
        _In_opt_ LPCWSTR,
        _Out_writes_opt_(cchDate) LPWSTR,
        _In_ int cchDate,
        _In_opt_ LPCWSTR);
    using get_date_format_func_t = int (WINAPI *)(
        _In_ LCID,
        _In_ DWORD,
        _In_opt_ CONST SYSTEMTIME*,
        _In_opt_ LPCWSTR,
        _Out_writes_opt_(cchDate) LPWSTR,
        _In_ int cchDate);

    get_locale_info_ex_func_t getLocaleInfoEx = nullptr;
    get_locale_info_func_t getLocaleInfo = nullptr;
    enum_date_formats_ex_ex_func_t enumDateFormatsExEx = nullptr;
    enum_date_formats_ex_func_t enumDateFormatsEx = nullptr;
    get_date_format_ex_func_t getDateFormatEx = nullptr;
    get_date_format_func_t getDateFormat = nullptr;

    bool vistaOrLater = false;
    HMODULE kernel32 = nullptr;
    if (GetModuleHandleEx(0, TEXT("kernel32.dll"), &kernel32) && kernel32)
    {
        getLocaleInfoEx = getFunction<get_locale_info_ex_func_t>(nullptr, "GetLocaleInfoEx", kernel32);
        enumDateFormatsExEx = getFunction<enum_date_formats_ex_ex_func_t>(nullptr, "EnumDateFormatsExEx", kernel32);
        getDateFormatEx = getFunction<get_date_format_ex_func_t>(nullptr, "GetDateFormatEx", kernel32);
        vistaOrLater = (getLocaleInfoEx && enumDateFormatsExEx && getDateFormatEx);
        if (!vistaOrLater)
        {
#if defined(_UNICODE)
            getLocaleInfo = getFunction<get_locale_info_func_t>(nullptr, "GetLocaleInfoW", kernel32);
            enumDateFormatsEx = getFunction<enum_date_formats_ex_func_t>(nullptr, "EnumDateFormatsExW", kernel32);
            getDateFormat = getFunction<get_date_format_func_t>(nullptr, "GetDateFormatW", kernel32);
#else
            getLocaleInfo = getFunction<get_locale_info_func_t>(nullptr, "GetLocaleInfoA", kernel32);
            enumDateFormatsEx = getFunction<enum_date_formats_ex_func_t>(nullptr, "EnumDateFormatsExA", kernel32);
            getDateFormat = getFunction<get_date_format_func_t>(nullptr, "GetDateFormatA", kernel32);
#endif
            // Well, the getLocaleInfo, enumDateFormatsEx and getDateFormat must be valid here. I don't check it.
        }
    }

    size_t notUsedResultLength;
    if (!stringLength)
    {
        stringLength = &notUsedResultLength;
    }
    bool result = false;
    if (vistaOrLater || (getLocaleInfo && enumDateFormatsEx && getDateFormat))
    {
        const LCTYPE localeDateFormat = useLongDateFormat ? LOCALE_SLONGDATE : LOCALE_SSHORTDATE;
        int formatLength =
            (getLocaleInfoEx ?
            (getLocaleInfoEx)(LOCALE_NAME_USER_DEFAULT, localeDateFormat, nullptr, 0) :
            (getLocaleInfo)(LOCALE_USER_DEFAULT, localeDateFormat, nullptr, 0));
        if (formatLength)
        {
            STLADD t_char_unique_ptr_t formatGuard {new TCHAR[formatLength]};
            LPTSTR format {formatGuard.get()};
            if (getLocaleInfoEx)
            {
                if (!(getLocaleInfoEx)(LOCALE_NAME_USER_DEFAULT, localeDateFormat, format, formatLength))
                {
                    formatGuard.reset();
                    format = nullptr;
                }
            }
            else
            {
                if (!(getLocaleInfo)(LOCALE_USER_DEFAULT, localeDateFormat, format, formatLength))
                {
                    formatGuard.reset();
                    format = nullptr;
                }
            }

            const DWORD flags = useLongDateFormat ? DATE_LONGDATE : DATE_SHORTDATE;
            if (searchForTwoDigitsFormat)
            {
                if (nullptr != format)
                {
                    // Check if the format formatting string has only two-digits year type.
                    const STLADD regex_type regexOfFormat {TEXT("(?:^yy[^y].*)|(?:.*[^y]yy[^y].*)|(?:.*[^y]yy$)")};
                    if (!std::regex_match(format, format + formatLength, regexOfFormat))
                    {
                        formatGuard.reset();
                        format = nullptr;
                    }
                }

                if (nullptr == format)
                {
                    /*
                     * Search other, non-default LOCALE_SSHORTDATE formatting string.
                     * The m_pszEnumDateFormatsProcExData data memeber is used by the
                     * `string_util::enumDateFormatsProcEx` static method.
                     */
                    m_enumDateFormatsProcExData = &format;
                    BOOL enumResult =
                        (enumDateFormatsExEx ?
                        (enumDateFormatsExEx)(enumDateFormatsProcExEx,
                                              LOCALE_NAME_USER_DEFAULT,
                                              flags,
                                              reinterpret_cast<LPARAM> (&format)) :
                        (enumDateFormatsEx)(enumDateFormatsProcEx, LOCALE_USER_DEFAULT, flags));
                    if (!enumResult && format)
                    {
                        formatGuard.reset();
                        format = nullptr;
                    }
                }
            }

            // Use found or standard formatting string to format the systemTime.
            const DWORD getDateFlags = format ? 0 : flags;
            if (getDateFormatEx)
            {
                *stringLength = (getDateFormatEx)(
                    LOCALE_NAME_USER_DEFAULT, getDateFlags, systemTime, format, nullptr, 0, nullptr);
            }
            else
            {
                *stringLength = (getDateFormat)(LOCALE_USER_DEFAULT, getDateFlags, systemTime, format, nullptr, 0);
            }
            if (*stringLength)
            {
                *string = new TCHAR[*stringLength];
                if (getDateFormatEx)
                {
                    *stringLength = (getDateFormatEx)(
                        LOCALE_NAME_USER_DEFAULT,
                        getDateFlags,
                        systemTime,
                        format,
                        *string,
                        static_cast<int> ((*stringLength)),
                        nullptr);
                }
                else
                {
                    *stringLength = (getDateFormat)(
                        LOCALE_USER_DEFAULT,
                        getDateFlags,
                        systemTime,
                        format,
                        *string,
                        static_cast<int> ((*stringLength)));
                }
                if (*stringLength)
                {
                    // Exclude NULL terminating character.
                    --(*stringLength);
                    result = true;
                }
                else
                {
                    // Error has occurred.
                    delete[] *string;
                    *string = nullptr;
                }
            }
        }
    }
    if (nullptr != kernel32)
    {
        FreeLibrary(kernel32);
    }

    return result;
}


/**
 * Converts SYSTEMTIME structure to string like "TIME_NOSECONDS".
 * This member uses Vista's (tm) GetTimeFormatEx when the application is running under Windows(R) 6 or higher,
 * or GetTimeFormat function for old systems.
 *
 * Parameters:
 * >systemTime
 * System time to be converted to string representation.
 * >string
 * Pointer to variable, that receives address of string buffer w/ result. This buffer is allocated by this method, and
 * when it completes its works successfully it is a caller responsibility to free that memory with `delete` operator.
 * But when the method fails a caller must ignore this and the next out parameters.
 * >stringLength
 * Pointer to variable that receives string result length not including NULL-terminating character.
 *
 * Returns:
 * true value in case of success, false -- otherwise.
 */
_Success_(return)
bool string_util::convertSystemTimeOnlyToString(
    _In_ const SYSTEMTIME* systemTime,
    _When_(0 != return, _Outptr_result_z_)
    _When_(0 == return, _Outptr_result_maybenull_z_) LPTSTR* const string,
    _Out_ size_t* stringLength) const
{
    using get_time_format_ex_func_t = int (WINAPI *)(
        _In_opt_ LPCWSTR,
        _In_ DWORD,
        _In_opt_ CONST SYSTEMTIME*,
        _In_opt_ LPCWSTR,
        _Out_writes_opt_(cchTime) LPWSTR,
        _In_ int cchTime);
    using get_time_format_func_t = int (WINAPI *)(
        _In_ LCID,
        _In_ DWORD,
        _In_opt_ CONST SYSTEMTIME*,
        _In_opt_ LPCWSTR,
        _Out_writes_opt_(cchTime) LPWSTR,
        _In_ int cchTime);

    bool result = false;
    get_time_format_func_t getTimeFormat = nullptr;
    HMODULE kernel32 = nullptr;
    get_time_format_ex_func_t getTimeFormatEx =
        getFunction<get_time_format_ex_func_t>(TEXT("kernel32.dll"), "GetTimeFormatEx", kernel32);
    if (!getTimeFormatEx)
    {
#if defined(_UNICODE)
        getTimeFormat = getFunction<get_time_format_func_t>(TEXT("kernel32.dll"), "GetTimeFormatW", kernel32);
#else
        getTimeFormat = getFunction<get_time_format_func_t>(TEXT("kernel32.dll"), "GetTimeFormatA", kernel32);
#endif
    }

    *stringLength = 0;
    if (getTimeFormatEx)
    {
        *stringLength = (getTimeFormatEx)(LOCALE_NAME_USER_DEFAULT, TIME_NOSECONDS, systemTime, nullptr, nullptr, 0);
    }
    else if (getTimeFormat)
    {
        // For system older than Vista(tm).
        *stringLength = (getTimeFormat)(LOCALE_USER_DEFAULT, TIME_NOSECONDS, systemTime, nullptr, nullptr, 0);
    }
    if (*stringLength)
    {
        *string = new TCHAR[*stringLength];
        if (getTimeFormatEx)
        {
            *stringLength = (getTimeFormatEx)(
                LOCALE_NAME_USER_DEFAULT,
                TIME_NOSECONDS,
                systemTime,
                nullptr,
                *string,
                static_cast<int> ((*stringLength)));
        }
        else if (getTimeFormat)
        {
            *stringLength = (getTimeFormat)(
                LOCALE_USER_DEFAULT,
                TIME_NOSECONDS,
                systemTime,
                nullptr,
                *string,
                static_cast<int> ((*stringLength)));
        }

        if (*stringLength)
        {
            // Exclude NULL terminating character.
            --(*stringLength);
            result = true;
        }
        else
        {
            // Error has occurred.
            delete[] *string;
            *string = nullptr;
        }
    }
    if (nullptr != kernel32)
    {
        FreeLibrary(kernel32);
    }

    return result;
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
    using get_number_format_ex_func_t = int (WINAPI *)(
        _In_opt_ LPCWSTR,
        _In_ DWORD,
        _In_ LPCWSTR,
        _In_opt_ CONST NUMBERFMTW*,
        _Out_writes_opt_(cchNumber) LPWSTR,
        _In_ int cchNumber);
    using get_number_format_func_t = int (WINAPI *)(
        _In_ LCID,
        _In_ DWORD,
        _In_ LPCTSTR,
        _In_opt_ CONST NUMBERFMT*,
        _Out_writes_opt_(cchNumber) LPTSTR,
        _In_ int cchNumber);

    STLADD string_unique_ptr_t result {};
    const size_t numberLength = 32;
    STLADD t_char_unique_ptr_t number {new TCHAR[numberLength]};
    numberLength = _stprintf_s(number, numberLength, printf_value_trait<T>::get(), value);

    /*
     * Call the GetNumberFormatEx or GetNumberFormat function.
     */
    get_number_format_ex_func_t getNumberFormatEx = nullptr;
    get_number_format_func_t getNumberFormat = nullptr;
    get_locale_info_ex_func_t getLocaleInfoEx = nullptr;
    get_locale_info_func_t getLocaleInfo = nullptr;

    HMODULE kernel32 = nullptr;
    if (GetModuleHandleEx(0, TEXT("kernel32.dll"), &kernel32) && kernel32)
    {
        getNumberFormatEx = getFunction<get_number_format_ex_func_t>(nullptr, "GetNumberFormatEx", kernel32);
        getLocaleInfoEx = getFunction<get_locale_info_ex_func_t>(nullptr, "GetLocaleInfoEx", kernel32);
        if (!(getNumberFormatEx && getLocaleInfoEx))
        {
#if defined(_UNICODE)
            getNumberFormat = getFunction<get_number_format_func_t>(nullptr, "GetNumberFormatW", kernel32);
            getLocaleInfo = getFunction<get_locale_info_func_t>(nullptr, "GetLocaleInfoW", kernel32);
#else
            getNumberFormat = getFunction<get_number_format_func_t>(nullptr, "GetNumberFormatA", kernel32);
            getLocaleInfo = getFunction<get_locale_info_func_t>(nullptr, "GetLocaleInfoA", kernel32);
#endif
            // Well, the getNumberFormat and getLocaleInfo must be valid here. I don't check it.
        }
    }

    if ((getNumberFormatEx && getLocaleInfoEx) || (getNumberFormat && getLocaleInfo))
    {
        NUMBERFMT numberFormat;
        /*
         * Fill the numberFormat structure.
         * Set number of fractional digits.
         */
#pragma warning(disable: 4127)
        if (std::is_floating_point<T>::value)
        {
            if (getLocaleInfoEx)
            {
                if (!(getLocaleInfoEx)(LOCALE_NAME_USER_DEFAULT,
                                       LOCALE_IDIGITS | LOCALE_RETURN_NUMBER,
                                       reinterpret_cast<LPTSTR> (&(numberFormat.NumDigits)),
                                       sizeof(numberFormat.NumDigits) / sizeof(TCHAR)))
                {
                    numberFormat.NumDigits = 2;
                }
            }
            else
            {
                if (!(getLocaleInfo)(LOCALE_USER_DEFAULT,
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
        if (getLocaleInfoEx)
        {
            if (!(getLocaleInfoEx)(LOCALE_NAME_USER_DEFAULT,
                                   LOCALE_ILZERO | LOCALE_RETURN_NUMBER,
                                   reinterpret_cast<LPTSTR> (&(numberFormat.LeadingZero)),
                                   sizeof(numberFormat.LeadingZero) / sizeof(TCHAR)))
            {
                numberFormat.LeadingZero = 1;
            }
        }
        else
        {
            if (!(getLocaleInfo)(LOCALE_USER_DEFAULT,
                                 LOCALE_ILZERO | LOCALE_RETURN_NUMBER,
                                 reinterpret_cast<LPTSTR> (&(numberFormat.LeadingZero)),
                                 sizeof(numberFormat.LeadingZero) / sizeof(TCHAR)))
            {
                numberFormat.LeadingZero = 1;
            }
        }
        // Set default (hard-coded) value to the numberFormat.Grouping.
        numberFormat.Grouping = 3;
        LPTSTR info;
        int infoLength =
            getLocaleInfoEx ?
            (getLocaleInfoEx)(LOCALE_NAME_USER_DEFAULT, LOCALE_SGROUPING, nullptr, 0) :
            (getLocaleInfo)(LOCALE_USER_DEFAULT, LOCALE_SGROUPING, nullptr, 0);
        if (infoLength)
        {
            STLADD t_char_unique_ptr_t guard {new TCHAR[(static_cast<size_t> (infoLength)) << 1]};
            info = guard.get();
            infoLength =
                getLocaleInfoEx ?
                (getLocaleInfoEx)(LOCALE_NAME_USER_DEFAULT, LOCALE_SGROUPING, info, infoLength) :
                (getLocaleInfo)(LOCALE_USER_DEFAULT, LOCALE_SGROUPING, info, infoLength);
            if (infoLength)
            {
                LPTSTR source = info;
                LPTSTR dest = info + infoLength;
                while (TEXT('\x00') != *source)
                {
                    // Skip ';' and '0' symbols at the end.

                    if ((TEXT(';') != *source) &&
                        ((TEXT('0') != *source) || (TEXT('\x00') != *(source + 1))))
                    {
                        *dest = *source;
                        ++dest;
                    }
                    ++source;
                }
                *dest = TEXT('\x00');

                numberFormat.Grouping = _tcstoul(info + infoLength, nullptr, 10);
            }
        }
        numberFormat.lpDecimalSep = nullptr;
        infoLength =
            getLocaleInfoEx ?
            (getLocaleInfoEx)(LOCALE_NAME_USER_DEFAULT, LOCALE_SDECIMAL, nullptr, 0) :
            (getLocaleInfo)(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, nullptr, 0);
        if (infoLength)
        {
            info = new TCHAR[infoLength];
            if (getLocaleInfoEx)
            {
                if ((getLocaleInfoEx)(LOCALE_NAME_USER_DEFAULT, LOCALE_SDECIMAL, info, infoLength))
                {
                    numberFormat.lpDecimalSep = info;
                }
                else
                {
                    delete[] info;
                }
            }
            else
            {
                if ((getLocaleInfo)(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, info, infoLength))
                {
                    numberFormat.lpDecimalSep = info;
                }
                else
                {
                    delete[] info;
                }
            }
        }
        numberFormat.lpThousandSep = nullptr;
        infoLength =
            getLocaleInfoEx ?
            (getLocaleInfoEx)(LOCALE_NAME_USER_DEFAULT, LOCALE_STHOUSAND, nullptr, 0) :
            (getLocaleInfo)(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, nullptr, 0);
        if (infoLength)
        {
            info = new TCHAR[infoLength];
            if (getLocaleInfoEx)
            {
                if ((getLocaleInfoEx)(LOCALE_NAME_USER_DEFAULT, LOCALE_STHOUSAND, info, infoLength))
                {
                    numberFormat.lpThousandSep = info;
                }
                else
                {
                    delete[] info;
                }
            }
            else
            {
                if ((getLocaleInfo)(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, info, infoLength))
                {
                    numberFormat.lpThousandSep = info;
                }
                else
                {
                    delete[] info;
                }
            }
        }
        if (getLocaleInfoEx)
        {
            if (!(getLocaleInfoEx)(LOCALE_NAME_USER_DEFAULT,
                                   LOCALE_INEGNUMBER | LOCALE_RETURN_NUMBER,
                                   reinterpret_cast<LPTSTR> (&(numberFormat.NegativeOrder)),
                                   sizeof(numberFormat.NegativeOrder) / sizeof(TCHAR)))
            {
                numberFormat.NegativeOrder = 1;
            }
        }
        else
        {
            if (!(getLocaleInfo)(LOCALE_USER_DEFAULT,
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
        int formattedNumberLength =
            getNumberFormatEx ?
            (getNumberFormatEx)(LOCALE_NAME_USER_DEFAULT, 0, number, &numberFormat, nullptr, 0) :
            (getNumberFormat)(LOCALE_USER_DEFAULT, 0, number, &numberFormat, nullptr, 0);
        if (formattedNumberLength)
        {
            STLADD t_char_unique_ptr_t guard {new TCHAR[formattedNumberLength]};
            LPTSTR formattedNumber = guard.get();
            if (getNumberFormatEx)
            {
                formattedNumberLength = (getNumberFormatEx)(
                    LOCALE_NAME_USER_DEFAULT, 0, number, &numberFormat, formattedNumber, formattedNumberLength);
            }
            else
            {
                formattedNumberLength = (getNumberFormat)(
                    LOCALE_USER_DEFAULT, 0, number, &numberFormat, formattedNumber, formattedNumberLength);
            }
            result = std::make_unique<STLADD string_type>(formattedNumber, formattedNumberLength - 1);
        }

        if (numberFormat.lpThousandSep)
        {
            delete[] numberFormat.lpThousandSep;
        }
        if (numberFormat.lpDecimalSep)
        {
            delete[] numberFormat.lpDecimalSep;
        }
    }
    if (nullptr != kernel32)
    {
        FreeLibrary(kernel32);
    }

    if (!result)
    {
        result = std::make_unique<STLADD string_type>(number, numberLength);
    }
    return result;
}


/**
 * A callback function for the EnumDateFormatsExEx function @ string_util::convertSystemDateOnlyToString method.
 *
 * Parameters:
 * >dateFormatString
 * Pointer to a buffer containing a null-terminated date format string. This string is a short date format.
 * >calendarID
 * Calendar identifier associated with the specified date format string.
 * >param
 * Pointer to a variable that should receive a pointer to application allocated memory.
 *
 * Returns:
 * TRUE to continue enumeration or FALSE otherwise.
 */
BOOL CALLBACK string_util::enumDateFormatsProcExEx(
    _In_z_ LPWSTR dateFormatString, _In_ CALID calendarID, _In_ LPARAM param)
{
    UNREFERENCED_PARAMETER(calendarID);
    BOOL result = TRUE;
    const STLADD regex_type regexOfFormat(TEXT("(?:^yy[^y].*)|(?:.*[^y]yy[^y].*)|(?:.*[^y]yy$)"));
    if (std::regex_match(dateFormatString, regexOfFormat))
    {
        size_t formatLength;
        if (SUCCEEDED(StringCchLength(dateFormatString, LOCALE_SSHORTDATE_MAX_LENGTH, &formatLength)))
        {
            LPTSTR* format = reinterpret_cast<LPTSTR*> (param);
            *format = new TCHAR[formatLength + 1];
            if (SUCCEEDED(StringCchCopy(*format, formatLength + 1, dateFormatString)))
            {
                result = FALSE;
            }
            else
            {
                delete[] *format;
                *format = nullptr;
            }
        }
    }
    return result;
}


/**
 * A callback function for the EnumDateFormatsExEx function @ string_util::convertSystemDateOnlyToString method.
 *
 * Parameters:
 * >dateFormatString
 * Pointer to a buffer containing a null-terminated date format string. This string is a short date format.
 * >calendarID
 * Calendar identifier associated with the specified date format string.
 *
 * Returns:
 * TRUE to continue enumeration or FALSE otherwise.
 */
BOOL CALLBACK string_util::enumDateFormatsProcEx(_In_z_ LPTSTR dateFormatString, _In_ CALID calendarID)
{
    return enumDateFormatsProcExEx(
        dateFormatString, calendarID, reinterpret_cast<LPARAM> (string_util::m_enumDateFormatsProcExData));
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
    get_locale_info_func_t getLocaleInfo = nullptr;
    HMODULE kernel32 = nullptr;
    get_locale_info_ex_func_t getLocaleInfoEx =
        getFunction<get_locale_info_ex_func_t>(TEXT("kernel32.dll"), "GetLocaleInfoEx", kernel32);
    if (!getLocaleInfoEx)
    {
#if defined(_UNICODE)
        getLocaleInfo = getFunction<get_locale_info_func_t>(nullptr, "GetLocaleInfoW", kernel32);
#else
        getLocaleInfo = getFunction<get_locale_info_func_t>(nullptr, "GetLocaleInfoA", kernel32);
#endif
        // Well, the getLocaleInfo must be valid here. I don't check it.
    }

    STLADD string_unique_ptr_t result {};

    if (getLocaleInfoEx || getLocaleInfo)
    {
        size_t listSeparatorLength;
        if (getLocaleInfoEx)
        {
            listSeparatorLength = (getLocaleInfoEx)(LOCALE_NAME_USER_DEFAULT, LOCALE_SLIST, nullptr, 0);
        }
        else
        {
            listSeparatorLength = (getLocaleInfo)(LOCALE_USER_DEFAULT, LOCALE_SLIST, nullptr, 0);
        }

        if (listSeparatorLength)
        {
            // Allocate memory to store list separator string only.
            STLADD t_char_unique_ptr_t guard {new TCHAR[listSeparatorLength]};
            LPTSTR rawResult {guard.get()};
            if (getLocaleInfoEx)
            {
                listSeparatorLength = (getLocaleInfoEx)(
                    LOCALE_NAME_USER_DEFAULT, LOCALE_SLIST, rawResult, static_cast<int> (listSeparatorLength));
            }
            else
            {
                listSeparatorLength = (getLocaleInfo)(
                    LOCALE_USER_DEFAULT, LOCALE_SLIST, rawResult, static_cast<int> (listSeparatorLength));
            }
            if (listSeparatorLength)
            {
                if (TEXT('\x20') != *(rawResult + listSeparatorLength - 2))
                {
                    // Replace NULL termintaing character w/ 'space' character.
                    *(rawResult + listSeparatorLength - 1) = TEXT('\x20');
                    result = std::make_unique<STLADD string_type>(rawResult, rawResult + listSeparatorLength);
                }
                else
                {
                    result = std::make_unique<STLADD string_type>(rawResult, rawResult + listSeparatorLength - 1);
                }
            }
        }
    }
    if (nullptr != kernel32)
    {
        FreeLibrary(kernel32);
    }

    if (!result)
    {
        result = std::make_unique<STLADD string_type>(DEFAULT_LIST_SEPARATOR, _countof(DEFAULT_LIST_SEPARATOR) - 1);
    }

    return result;
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
    get_locale_info_func_t getLocaleInfo = nullptr;
    HMODULE kernel32 = nullptr;
    get_locale_info_ex_func_t getLocaleInfoEx =
        getFunction<get_locale_info_ex_func_t>(TEXT("kernel32.dll"), "GetLocaleInfoEx", kernel32);
    if (!getLocaleInfoEx)
    {
#if defined(_UNICODE)
        getLocaleInfo = getFunction<get_locale_info_func_t>(nullptr, "GetLocaleInfoW", kernel32);
#else
        getLocaleInfo = getFunction<get_locale_info_func_t>(nullptr, "GetLocaleInfoA", kernel32);
#endif
        // Well, the getLocaleInfo must be valid here. I don't check it.
    }

    STLADD string_unique_ptr_t result {};

    if (getLocaleInfoEx || getLocaleInfo)
    {
        size_t length;
        if (getLocaleInfoEx)
        {
            length = (getLocaleInfoEx)(LOCALE_NAME_USER_DEFAULT, LOCALE_SNAME, nullptr, 0);
        }
        else
        {
            length = (getLocaleInfo)(LOCALE_USER_DEFAULT, LOCALE_SNAME, nullptr, 0);
        }

        if (length)
        {
            // Allocate memory to store list separator string only.
            STLADD t_char_unique_ptr_t guard {new TCHAR[length]};
            LPTSTR rawResult {guard.get()};
            if (getLocaleInfoEx)
            {
                length =
                    (getLocaleInfoEx)(LOCALE_NAME_USER_DEFAULT, LOCALE_SNAME, rawResult, static_cast<int> (length));
            }
            else
            {
                length = (getLocaleInfo)(LOCALE_USER_DEFAULT, LOCALE_SNAME, rawResult, static_cast<int> (length));
            }
            if (length)
            {
                result = std::make_unique<STLADD string_type>(rawResult, rawResult + length - 1);
            }
        }
    }
    if (nullptr != kernel32)
    {
        FreeLibrary(kernel32);
    }

    return result;
}
#pragma endregion string_util class
