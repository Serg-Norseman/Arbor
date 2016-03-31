#pragma once
#include "ns\arbor.h"
#include "ns\stladd.h"
#include "service\winapi\heap.h"
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

ARBOR_INLINE_BEGIN

#pragma region memory
class private_heap
{
protected:
    HANDLE getHeap() const noexcept
    {
        class initializer
        {
        public:
            initializer()
            {
                /*
                 * C++11 guarantees thread-safety here (static local variables initialization, [6.7]).
                 *
                 * `m_heap` is protected from multiple access by Windows (see implementation of `createHeap` method).
                 */
                m_heap.reset(createHeap());
            }
        };
        static initializer guard {};
        return m_heap.get();
    }


private:
    static HANDLE createHeap();

    // Do not access `m_heap` directly. Use `getHeap` method instead.
    static WAPI heap_t m_heap;
};

/**
 * class default_allocator.
 * Allocator for STL.
 */
template <typename T>
class default_allocator: private private_heap
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

    default_allocator() = default;
    default_allocator(_In_ const default_allocator&) = default;

    template <typename U>
    default_allocator(_In_ const default_allocator<U>&) noexcept
    {
    }

    default_allocator& operator =(_In_ const default_allocator&) = default;

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

    pointer allocate(_In_ const size_type count) const noexcept(false)
    {
        if (count)
        {
            if (max_size() < count)
            {
                throw std::length_error {"`default_allocator::allocate`, length too long"};
            }
            void* p = HeapAlloc(getHeap(), 0, count * sizeof(value_type));
            if (!p)
            {
                throw std::bad_alloc {};
            }
            return static_cast<pointer> (p);
        }
        else
        {
            return nullptr;
        }
    }

    pointer allocate(_In_ const size_type size, _In_ const size_type) const noexcept(false)
    {
        return allocate(size / sizeof(value_type));
    }

    template <typename U>
    pointer allocate(_In_ const size_type count, _In_ typename U::const_pointer) const noexcept(false)
    {
        return allocate(count);
    }

    void deallocate(_In_ const pointer p, _In_ const size_type) const
    {
        HeapFree(getHeap(), 0, p);
    }

    void deallocate(_In_ void* p) const
    {
        deallocate(static_cast<pointer> (p), 0);
    }
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


/*
 * The `maxAlignment` function can be a member of `aligned_allocator` **template**, but because it looks like ICC 16.0
 * has a bug (it doesn't allow to use a constexpr member function of a template class inside this template class body),
 * I have to move declaration and definition of the `maxAlignment` function out of `aligned_allocator` template.
 * (Note: MSVC 2015 Update 1 can compile such a template instantiation).
 *
 * For more information please view the following pages:
 * stackoverflow.com/questions/35764069/static-assert-and-intel-c-compiler
 * and
 * stackoverflow.com/questions/16493652/constexpr-not-working-if-the-function-is-declared-inside-class-scope
 *
 * Also view 'compiler_bugs.md' file.
 */
template <typename T, size_t Alignment>
constexpr size_t maxAlignment()
{
    return max(std::alignment_of<T>::value, Alignment);
}

/**
 * `aligned_allocator` aligns allocated memory on the maximum of the minimum alignment specified and the alignment of
 * objects of type `T`.
 */
template <typename T, size_t Alignment>
class aligned_allocator: private private_heap
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
    typedef std::integral_constant<size_t, maxAlignment<value_type, Alignment>()> max_alignment_t;

    template <typename U>
    struct rebind
    {
        typedef aligned_allocator<U, Alignment> other;
    };

    aligned_allocator() = default;
    aligned_allocator(_In_ const aligned_allocator&) = default;

    template <typename U>
    aligned_allocator(_In_ const aligned_allocator<U, Alignment>&) noexcept
    {
    }

    aligned_allocator& operator =(_In_ const aligned_allocator&) = default;

    template <typename U, size_t UAlignment>
    constexpr bool operator ==(_In_ const aligned_allocator<U, UAlignment>& right) const noexcept
    {
        return max_alignment_t::value == aligned_allocator<U, UAlignment>::max_alignment_t::value;
    }

    template <typename U, size_t UAlignment>
    constexpr bool operator !=(_In_ const aligned_allocator<U, UAlignment>& right) const noexcept
    {
        return !(*this == right);
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
        return (static_cast<size_type> (~0) - (max_alignment_t::value - 1 + sizeof(pointer))) / sizeof(value_type);
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

    pointer allocate(_In_ const size_type count) const noexcept(false)
    {
        if (count)
        {
            // `max_size` takes into account the additional space for alignment and "real" pointer.
            if (max_size() < count)
            {
                throw std::length_error {"`aligned_allocator::allocate`, length too long"};
            }
            /*
             * `sizeof(value_type)` is always greater or equal to `maxAlignment`, therefore, if `count` is more
             * than `1`, the second and all following objects will be properly aligned on a `maxAlignment`-byte
             * boundary.
             *
             * When do I get more "pure C++" code: using `sizeof(pointer)` or `sizeof(size_t)`?
             */
            size_t padding = max_alignment_t::value - 1 + sizeof(pointer);
            void* p = HeapAlloc(getHeap(), 0, count * sizeof(value_type) + padding);
            if (!p)
            {
                throw std::bad_alloc {};
            }
            auto aligned =
                reinterpret_cast<size_t*> (((reinterpret_cast<size_t> (p)) + padding) & ~(max_alignment_t::value - 1));
            *(aligned - 1) = reinterpret_cast<size_t> (p);
            return reinterpret_cast<pointer> (aligned);
        }
        else
        {
            return nullptr;
        }
    }

    pointer allocate(_In_ const size_type size, _In_ const size_type) const noexcept(false)
    {
        return allocate(size / sizeof(value_type));
    }

    template <typename U>
    pointer allocate(_In_ const size_type count, _In_ typename U::const_pointer) const noexcept(false)
    {
        return allocate(count);
    }

    void deallocate(_In_ const pointer p, _In_ const size_type) const
    {
        HeapFree(getHeap(), 0, reinterpret_cast<void*> (*((reinterpret_cast<size_t*> (p)) - 1)));
    }

    void deallocate(_In_ void* p) const
    {
        deallocate(static_cast<pointer> (p), 0);
    }

    static_assert(0 == ((max_alignment_t::value - 1) & max_alignment_t::value),
        "Maximum of `Alignment` and alignment of `T` must be a power of 2");
};


template<size_t Alignment>
class aligned_allocator<void, Alignment>
{
public:
    typedef void value_type;
    typedef void* pointer;
    typedef const void* const_pointer;

    template<class U>
    struct rebind
    {
        typedef aligned_allocator<U, Alignment> other;
    };
};

template <typename T>
using aligned_sse_allocator = aligned_allocator<T, alignof(__m128)>;



template <typename T, size_t Alignment>
class aligned_deleter
{
public:
    typedef typename std::conditional<std::is_pointer<T>::value, typename std::remove_pointer<T>::type, T>::type
        element_type;
    typedef typename std::conditional<std::is_pointer<T>::value, T, typename std::add_pointer<T>::type>::type pointer;

    void operator ()(_In_ pointer p)
    {
        aligned_allocator<element_type, Alignment> allocator {};
        allocator.destroy(p);
        allocator.deallocate(p);
    }
};

template <typename T>
using aligned_sse_deleter = aligned_deleter<T, alignof(__m128)>;
#pragma endregion memory resource handling

#pragma region typedefs
typedef std::unique_ptr<LOGFONT> logfont_unique_ptr_t;
typedef std::unique_ptr<unsigned char> u_char_unique_ptr_t;
typedef std::unique_ptr<TCHAR> t_char_unique_ptr_t;

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

ARBOR_END

STLADD_END

inline STLADD string_type operator "" _s(STLADD string_type::const_pointer psz, size_t size)
{
    return STLADD string_type {psz, size};
}
