#pragma once
#include "ns\stladd.h"
#include "service\memorymg.h"
#include <functional>
#include <iterator>
#include <malloc.h>
#include <memory>
#include <mutex>
#include <regex>
#include <sstream>
#include <string>
#include <stdexcept>
#include <type_traits>
#include <Windows.h>
#include <Wincrypt.h>

STLADD_BEGIN

#pragma region memory
/**
 * class default_allocator.
 * Allocator for STL.
 */
template <typename T>
class default_allocator
{
public:
    typedef T value_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T&& rvalue_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    template <typename U>
    struct rebind
    {
        typedef default_allocator<U> other;
    };

    default_allocator() noexcept
        :
        m_nThreadId {GetCurrentThreadId()}
    {
    }

    default_allocator(_In_ const default_allocator& allocator) noexcept
        :
        m_nThreadId {allocator.m_nThreadId}
    {
    }

    template <typename U>
    default_allocator(_In_ const default_allocator<U>& allocator) noexcept
        :
        m_nThreadId {allocator.getThreadId()}
    {
    }

    default_allocator& operator =(_In_ const default_allocator&) noexcept
    {
        // Assign from an allocator (do nothing).
        return *this;
    }

    template <typename U>
    bool operator ==(_In_ const default_allocator<U>&) const noexcept
    {
        return true;
    }

    template <typename U>
    bool operator !=(_In_ const default_allocator<U>& other) const noexcept
    {
        return !(*this == other);
    }

    pointer address(_In_ reference value) const noexcept
    {
        return std::addressof(value);
    }

    const_pointer address(_In_ const_reference value) const noexcept
    {
        return std::addressof(value);
    }

    size_t max_size() const noexcept
    {
        return (static_cast<size_type> (~0)) / sizeof(value_type);
    }

    template <typename U, typename... Args>
    void construct(_In_ U* p, _In_ Args&&... args) const
    {
        void* pv = static_cast<void*> (p);
        new (pv) U {std::forward<Args>(args)...};
    }

    template <typename U>
    void construct(_In_ U* p) const
    {
        void* pv = static_cast<void*> (p);
        new (pv) U {};
    }

__pragma(warning(push)) __pragma(warning(disable: 4100))
    void destroy(_In_ const pointer p) const
    {
        p->~T();
    }
__pragma(warning(pop))

    pointer allocate(_In_ const size_type nCount) const throw(...)
    {
        if (!nCount)
        {
            return nullptr;
        }
        if (max_size() < nCount)
        {
            throw std::length_error {"default_allocator<T>::allocate() - Length too long."};
        }
        memory_manager::pointer_t pMemoryManager = memory_manager::getInstance();
        void* p = pMemoryManager->mAlloc<void*>(m_nThreadId, nCount * sizeof(value_type));
        if (!p)
        {
            throw std::bad_alloc();
        }
        return static_cast<pointer> (p);
    }

    template <typename U>
    pointer allocate(_In_ const size_type nCount, _In_ typename U::const_pointer) const
    {
        return allocate(nCount);
    }

    void deallocate(_In_ const pointer p, _In_ const size_type) const
    {
        memory_manager::pointer_t pMemoryManager = memory_manager::getInstance();
        pMemoryManager->free<void*>(m_nThreadId, p);
    }

    inline DWORD getThreadId() const
    {
        return m_nThreadId;
    }


private:
    // Thread whose heap will be used for allocation/deallocation.
    const DWORD m_nThreadId;
};


template<>
class default_allocator<void>
{
public:
    typedef void value_type;
    typedef void* pointer;
    typedef const void* const_pointer;

    template<class U>
    struct rebind
    {
        typedef default_allocator<U> other;
    };
};



/**
 * smart_ptr_deleter class.
 * Deleter that uses memory_manager service.
 */
template <typename T>
class smart_ptr_deleter
{
public:
    typedef typename std::conditional<std::is_pointer<T>::value, typename std::remove_pointer<T>::type, T>::type
        element_type;
    typedef typename std::conditional<std::is_pointer<T>::value, T, typename std::add_pointer<T>::type>::type pointer;

    smart_ptr_deleter() noexcept
        :
        m_nThreadId {GetCurrentThreadId()}
    {
    }

    smart_ptr_deleter(_In_ const smart_ptr_deleter&) = delete;

    smart_ptr_deleter(_In_ smart_ptr_deleter&& right) noexcept
        :
        m_nThreadId {right.m_nThreadId}
    {
    }

    void operator ()(_In_ pointer pMemory)
    {
        memory_manager::pointer_t pMemoryManager = memory_manager::getInstance();
        pMemoryManager->free<pointer>(m_nThreadId, pMemory);
    }

    smart_ptr_deleter& operator =(_In_ const smart_ptr_deleter& right) = delete;

    smart_ptr_deleter& operator =(_In_ smart_ptr_deleter&& right) noexcept
    {
        if (this != &right)
        {
            m_nThreadId = right.m_nThreadId;
        }
        return *this;
    }

    void setThreadId(_In_ const DWORD nThreadId) noexcept
    {
        m_nThreadId = nThreadId;
    }


protected:
    // Thread whose heap will be used for deallocation.
    DWORD m_nThreadId;
};



template <typename T>
class aligned_deleter
{
public:
    typedef typename std::conditional<std::is_pointer<T>::value, typename std::remove_pointer<T>::type, T>::type
        element_type;
    typedef typename std::conditional<std::is_pointer<T>::value, T, typename std::add_pointer<T>::type>::type pointer;

    void operator ()(_In_ pointer p)
    {
        _aligned_free(p);
    }
};



/**
 * CSharedPtrCryptoAPIBlobDeleter class.
 * Deleter for DATA_BLOB structure.
 * Derived from the 'smart_ptr_deleter' to get deallocation thread awareness.
 */
class data_blob_deleter: public smart_ptr_deleter<DATA_BLOB>
{
public:
    void operator ()(_In_ pointer pMemory)
    {
        memory_manager::pointer_t pMemoryManager = memory_manager::getInstance();
        if (pMemory->pbData)
        {
            pMemoryManager->free(m_nThreadId, pMemory->pbData);
        }
        pMemoryManager->free<pointer>(m_nThreadId, pMemory);
    }
};
#pragma endregion memory resource handling

#pragma region typedefs
typedef std::unique_ptr<DATA_BLOB, data_blob_deleter> data_blob_unique_ptr_t;
typedef std::shared_ptr<DATA_BLOB> data_blob_shared_ptr_t;
typedef std::unique_ptr<SYSTEMTIME, smart_ptr_deleter<SYSTEMTIME>> system_time_unique_ptr_t;
typedef std::unique_ptr<LOGFONT, smart_ptr_deleter<LOGFONT>> logfont_unique_ptr_t;
typedef std::unique_ptr<unsigned char, smart_ptr_deleter<unsigned char>> u_char_unique_ptr_t;
typedef std::unique_ptr<TCHAR, smart_ptr_deleter<TCHAR>> t_char_unique_ptr_t;

typedef std::basic_string<wchar_t, std::char_traits<wchar_t>, default_allocator<wchar_t>> w_string_type;
typedef std::basic_string<char, std::char_traits<char>, default_allocator<char>> a_string_type;
#if defined(_UNICODE)
    typedef w_string_type string_type;
#else
    typedef a_string_type string_type;
#endif
typedef std::unique_ptr<string_type> string_unique_ptr_t;

typedef std::basic_istringstream<wchar_t, std::char_traits<wchar_t>, default_allocator<wchar_t>> wistringstream;
typedef std::basic_ostringstream<wchar_t, std::char_traits<wchar_t>, default_allocator<wchar_t>> wostringstream;
typedef std::ostream_iterator<string_type, wchar_t> ostream_iterator;
typedef std::regex_token_iterator<string_type::const_iterator> sregex_token_iterator;
typedef std::regex_token_iterator<string_type::const_pointer> cregex_token_iterator;
typedef std::match_results<
    string_type::iterator, default_allocator<std::sub_match<string_type::iterator>>> match_results;
typedef std::match_results<
    string_type::const_iterator, default_allocator<std::sub_match<string_type::const_iterator>>> const_match_results;
typedef std::sub_match<string_type::const_iterator> ssub_match;
#if defined(_UNICODE)
    typedef std::wregex regex_type;
    typedef std::match_results<const wchar_t*> cmatch_results;
#else
    typedef std::regex regex_type;
    typedef std::match_results<const char*> cmatch_results;
#endif
#pragma endregion typedefs

#pragma region hash
template <typename T>
class smart_ptr_hash: public std::unary_function<T, size_t>
{
public:
    result_type operator ()(_In_ const argument_type& value) const
    {
        if (value)
        {
            std::hash<T::element_type> hasher;
            return hasher(*value);
        }
        else
        {
            return 0;
        }
    }
};

template <typename T>
class smart_ptr_equal_to: public std::binary_function<T, T, bool>
{
public:
    static_assert(
        std::is_same<first_argument_type, second_argument_type>::value,
        "The 'first_argument_type' and the 'second_argument_type' must be the same.");
    result_type operator ()(_In_ const first_argument_type& left, _In_ const second_argument_type& right) const
    {
        if (left && right)
        {
            std::equal_to<T::element_type> equal;
            return equal(*left, *right);
        }
        else
        {
            std::equal_to<first_argument_type> equal;
            return equal(left, right);
        }
    }
};
#pragma endregion types for hash maps

#pragma region locks
template<typename Mutex>
class lock_guard_exclusive
{
public:
    typedef Mutex mutex_type;

    _When_(this->m_bOwns, _Acquires_exclusive_lock_(this->m_mutex))
    explicit lock_guard_exclusive(_In_ mutex_type& mutex)
        :
        m_mutex {mutex},
        m_bOwns {false}
    {
        m_mutex.lockExclusive();
        m_bOwns = true;
    }

    _Requires_exclusive_lock_held_(mutex)
    lock_guard_exclusive(_In_ mutex_type& mutex, std::adopt_lock_t)
        :
        m_mutex {mutex},
        m_bOwns {true}
    {
    }

    _When_(this->m_bOwns, _Acquires_exclusive_lock_(this->m_mutex))
    lock_guard_exclusive(_In_ mutex_type& mutex, std::try_to_lock_t)
        :
        m_mutex {mutex},
        m_bOwns {m_mutex.tryLockExclusive()}
    {
    }

    _When_(this->m_bOwns, _Releases_exclusive_lock_(this->m_mutex))
    ~lock_guard_exclusive() noexcept
    {
        if (m_bOwns)
        {
            m_mutex.unlockExclusive();
        }
    }

    lock_guard_exclusive() = delete;
    lock_guard_exclusive(_In_ const lock_guard_exclusive&) = delete;
    lock_guard_exclusive& operator =(_In_ const lock_guard_exclusive&) = delete;
    // Declaration of a move ctor is omitted 'cos (12.8.9.1) and (12.8.9.2) of C++ std.
    // Declaration of a move assignment is omitted 'cos (12.8.20.1) and (12.8.20.3) of C++ std.

    explicit operator bool() const noexcept
    {
        return m_bOwns;
    }


private:
    mutex_type& m_mutex;
    bool m_bOwns = false;
};

template<typename Mutex>
class lock_guard_shared
{
public:
    typedef Mutex mutex_type;

    _When_(this->m_bOwns, _Acquires_exclusive_lock_(this->m_mutex))
    explicit lock_guard_shared(_In_ mutex_type& mutex)
        :
        m_mutex {mutex},
        m_bOwns {false}
    {
        m_mutex.lockShared();
        m_bOwns = true;
    }

    _Requires_shared_lock_held_(mutex)
    lock_guard_shared(_In_ mutex_type& mutex, std::adopt_lock_t)
        :
        m_mutex {mutex},
        m_bOwns {true}
    {
    }

    _When_(this->m_bOwns, _Acquires_exclusive_lock_(this->m_mutex))
        lock_guard_shared(_In_ mutex_type& mutex, std::try_to_lock_t)
        :
        m_mutex {mutex},
        m_bOwns {m_mutex.tryLockShared()}
    {
    }

    _When_(this->m_bOwns, _Releases_exclusive_lock_(this->m_mutex))
    ~lock_guard_shared() noexcept
    {
        if (m_bOwns)
        {
            m_mutex.unlockShared();
        }
    }

    lock_guard_shared() = delete;
    lock_guard_shared(_In_ const lock_guard_shared&) = delete;
    lock_guard_shared& operator =(_In_ const lock_guard_shared&) = delete;

    explicit operator bool() const noexcept
    {
        return m_bOwns;
    }


private:
    mutex_type& m_mutex;
    bool m_bOwns = false;
};
#pragma endregion locks

STLADD_END

inline STLADD string_type operator "" _s(STLADD string_type::const_pointer psz, size_t nSize)
{
    return STLADD string_type {psz, nSize};
}
