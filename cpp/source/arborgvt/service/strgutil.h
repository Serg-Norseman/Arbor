#pragma once
#include "service\stladdon.h"
#include <memory>

class string_util
{
public:
    typedef const string_util* const_ptr_t;
    typedef string_util* pointer_t;

    string_util()
        :
        m_moduleWithResource {nullptr}
    {
    }

    static pointer_t getInstance() noexcept(false);

    void setModuleHandleWithResource(_In_ HINSTANCE moduleWithResource)
    {
        if (moduleWithResource != m_moduleWithResource)
        {
            m_moduleWithResource = moduleWithResource;
        }
    }

    _Success_(return) bool loadString(
        _In_ UINT stringResourceId,
        _Out_writes_opt_z_(bufferLength) LPTSTR allocatedBuffer,
        _In_ size_t bufferLength) const;
    _Success_(return) bool loadString(
        _In_ UINT stringResourceId,
        _When_(0 != return, _Outptr_result_z_)
        _When_(0 == return, _Outptr_result_maybenull_z_) LPTSTR* resourceString,
        _Out_opt_ size_t* resourceLength) const;
    STLADD string_unique_ptr_t loadString(_In_ UINT stringResourceId) const;
    _Success_(return) bool loadReadOnlyString(
        _In_ UINT stringResourceId,
        _Outptr_result_z_ LPCTSTR* resourceString,
        _Out_ size_t* resourceLength) const;

    _Success_(return) bool formatOneStringField(
        _In_ UINT resultFormatId,
        _In_z_ LPCTSTR argument,
        _In_ size_t argumentLength,
        _When_(0 != return, _Outptr_result_z_)
        _When_(0 == return, _Outptr_result_maybenull_z_) LPTSTR* string,
        _Out_opt_ size_t* resultLength) const;

    _Success_(return) bool convertSystemTimeToString(
        _In_ const SYSTEMTIME* systemTime,
        _In_ const bool useLongDateFormat,
        _In_ const bool searchForTwoDigitsFormat,
        _When_(0 != return, _Outptr_result_z_)
        _When_(0 == return, _Outptr_result_maybenull_z_) LPTSTR* const string,
        _Out_opt_ size_t* stringLength) const;
    _Success_(return) bool convertSystemDateOnlyToString(
        _In_ const SYSTEMTIME* systemTime,
        _In_ const bool useLongDateFormat,
        _In_ const bool searchForTwoDigitsFormat,
        _When_(0 != return, _Outptr_result_z_)
        _When_(0 == return, _Outptr_result_maybenull_z_) LPTSTR* const string,
        _Out_opt_ size_t* stringLength) const;
    _Success_(return) bool convertSystemTimeOnlyToString(
        _In_ const SYSTEMTIME* systemTime,
        _When_(0 != return, _Outptr_result_z_)
        _When_(0 == return, _Outptr_result_maybenull_z_) LPTSTR* const string,
        _Out_ size_t* stringLength) const;

    template <typename T> _Check_return_ STLADD string_unique_ptr_t formatNumber(_In_ const T value) const;
    STLADD string_unique_ptr_t getListSeparator() const;
    _Check_return_ STLADD string_unique_ptr_t getCurrentUserLocale() const;


private:
    typedef std::unique_ptr<string_util> unique_ptr_t;

    static BOOL CALLBACK enumDateFormatsProcExEx(
        _In_z_ LPWSTR dateFormatString, _In_ CALID calendarID, _In_ LPARAM param);
    static BOOL CALLBACK enumDateFormatsProcEx(_In_z_ LPTSTR dateFormatString, _In_ CALID calendarID);

    static unique_ptr_t m_instance;
    static LPTSTR* m_enumDateFormatsProcExData;
    HINSTANCE m_moduleWithResource;
};
