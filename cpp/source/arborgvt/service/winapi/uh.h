#pragma once
#include "ns\wapi.h"
#include <sal.h>
#include <utility>

using std::swap;

template <typename T>
void swap_with_ADL(_In_ T& left, _In_ T& right) noexcept
{
    swap(left, right);
}

WAPI_BEGIN

template <typename Traits>
class unique_handle
{
public:
    typedef Traits traits_type;
    typedef decltype(traits_type::invalid()) handle_type;

    /*
     * Copy ctor and copy assignment operator ain't defined.
     */
    unique_handle(_In_ const unique_handle&) = delete;

    unique_handle() noexcept
        :
        m_handle {traits_type::invalid()}
    {
    }

    unique_handle(_In_ unique_handle&& right) noexcept
        :
        m_handle {right.release()}
    {
    }

    explicit unique_handle(_In_opt_ const handle_type handle) noexcept
        :
        m_handle {handle}
    {
    }

    ~unique_handle() noexcept
    {
        close();
    }

    unique_handle& operator =(_In_ const unique_handle&) = delete;

    unique_handle& operator =(_In_ unique_handle&& right) noexcept
    {
        if (&right != this)
        {
            reset(right.release());
        }
        return *this;
    }

    void swap(_In_ unique_handle& right) noexcept
    {
        swap_with_ADL(m_handle, right.m_handle);
    }

    explicit operator bool() const noexcept
    {
        return (traits_type::invalid() != m_handle);
    }

    bool operator ==(_In_ const unique_handle& right) const noexcept
    {
        return (right.m_handle == m_handle);
    }

    bool operator !=(_In_ const unique_handle& right) const noexcept
    {
        return (right.m_handle != m_handle);
    }

    handle_type get() const noexcept
    {
        return m_handle;
    }

    void reset() noexcept
    {
        close();
    }

    bool reset(_In_opt_ const handle_type handle) noexcept
    {
        if (handle != m_handle)
        {
            close();
            m_handle = handle;
        }
        return static_cast<bool> (*this);
    }

    handle_type release() noexcept
    {
        handle_type handle = m_handle;
        m_handle = traits_type::invalid();
        return handle;
    }


private:
    void close() noexcept
    {
        if (*this)
        {
            traits_type::close(m_handle);
            m_handle = traits_type::invalid();
        }
    }

    handle_type m_handle;
};

template <typename T>
class unique_handle_traits
{
public:
    typedef T type;

    static type invalid() noexcept
    {
        return nullptr;
    }
};

WAPI_END

template <typename Traits>
void swap(_In_ WAPI unique_handle<Traits>& left, _In_ WAPI unique_handle<Traits>& right) noexcept
{
    left.swap(right);
}
