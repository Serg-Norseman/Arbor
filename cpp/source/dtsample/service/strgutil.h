#pragma once
#include "service\stladdon.h"
#include "service\winapi\srwlock.h"
#include <memory>

class string_util
{
public:
    typedef const string_util* const_ptr_t;
    typedef string_util* pointer_t;

    static pointer_t getInstance() throw(...);

    void setModuleHandleWithResource(_In_ HINSTANCE hModuleWithResource)
    {
        if (hModuleWithResource != m_hModuleWithResource)
        {
            m_hModuleWithResource = hModuleWithResource;
        }
    }

    _Success_(return) bool loadString(
        _In_ UINT nStringResourceId,
        _Out_writes_opt_z_(nBufferLength) LPTSTR pszAllocatedBuffer,
        _In_ size_t nBufferLength) const;
    _Success_(return) bool loadString(
        _In_ UINT nStringResourceId,
        _When_(0 != return, _Outptr_result_z_)
        _When_(0 == return, _Outptr_result_maybenull_z_) LPTSTR* ppszResourceString,
        _Out_opt_ size_t* pnResourceLength) const;
    STLADD string_unique_ptr_t loadString(_In_ UINT nStringResourceId) const;
    _Success_(return) bool loadReadOnlyString(
        _In_ UINT nStringResourceId,
        _Outptr_result_z_ LPCTSTR* ppszResourceString,
        _Out_ size_t* pnResourceLength) const;
    void freeString(__drv_freesMem(Mem) _Frees_ptr_ LPTSTR pszResourceString) const;

    _Success_(return) bool formatOneStringField(
        _In_ UINT nResultFormatId,
        _In_z_ LPCTSTR pszArgument,
        _In_ size_t nArgumentLength,
        _When_(0 != return, _Outptr_result_z_)
        _When_(0 == return, _Outptr_result_maybenull_z_) LPTSTR* ppszResult,
        _Out_opt_ size_t* pnResultLength) const;

    _Success_(return) bool convertSystemTimeToString(
        _In_ const SYSTEMTIME* pSystemTime,
        _In_ const bool bUseLongDateFormat,
        _In_ const bool bSearchForTwoDigitsFormat,
        _When_(0 != return, _Outptr_result_z_)
        _When_(0 == return, _Outptr_result_maybenull_z_) LPTSTR* const ppszString,
        _Out_opt_ size_t* pnStringLength) const;
    _Success_(return) bool convertSystemDateOnlyToString(
        _In_ const SYSTEMTIME* pSystemTime,
        _In_ const bool bUseLongDateFormat,
        _In_ const bool bSearchForTwoDigitsFormat,
        _When_(0 != return, _Outptr_result_z_)
        _When_(0 == return, _Outptr_result_maybenull_z_) LPTSTR* const ppszString,
        _Out_opt_ size_t* pnStringLength) const;
    _Success_(return) bool convertSystemTimeOnlyToString(
        _In_ const SYSTEMTIME* pSystemTime,
        _When_(0 != return, _Outptr_result_z_)
        _When_(0 == return, _Outptr_result_maybenull_z_) LPTSTR* const ppszString,
        _Out_ size_t* pnStringLength) const;

    template <typename T> _Check_return_ STLADD string_unique_ptr_t formatNumber(_In_ const T value) const;
    STLADD string_unique_ptr_t getListSeparator() const;
    _Check_return_ STLADD string_unique_ptr_t getCurrentUserLocale() const;


private:
    typedef std::unique_ptr<string_util> unique_ptr_t;

    string_util()
        :
        m_hModuleWithResource(nullptr)
    {
    }

    static BOOL CALLBACK enumDateFormatsProcExEx(_In_z_ LPWSTR pszDateFormatString,
                                                 _In_ CALID nCalendarID,
                                                 _In_ LPARAM nParam);
    static BOOL CALLBACK enumDateFormatsProcEx(_In_z_ LPTSTR pszDateFormatString, _In_ CALID nCalendarID);

    static unique_ptr_t m_instance;
    static WAPI srw_lock m_instanceLock;
    static LPTSTR* m_ppszEnumDateFormatsProcExData;
    HINSTANCE m_hModuleWithResource;
};
