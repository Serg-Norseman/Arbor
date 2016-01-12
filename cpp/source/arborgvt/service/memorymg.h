#pragma once
#include "service\winapi\heap.h"
#include "service\winapi\srwlock.h"
#include <memory>
#include <unordered_map>
#include <Windows.h>

/**
 * Wrapper class for private heap object.
 */
class private_heap
{
public:
    private_heap(_In_ const private_heap&) = delete;
    private_heap& operator =(_In_ const private_heap&) = delete;

    private_heap(_In_ private_heap&& right) noexcept
        :
        m_heap(std::move(right.m_heap))
    {
    }

    explicit private_heap(_In_ size_t nHeapInitialSize);

    private_heap& operator =(_In_ private_heap&& right) noexcept
    {
        if (&right != this)
        {
            m_heap = std::move(right.m_heap);
        }
        return *this;
    }

    const HANDLE getHeap() const noexcept
    {
        return m_heap.get();
    }


private:
    WAPI heap_t m_heap;
};



/**
 * Simple memory manager that uses per-thread Windows private heap objects.
 */
class memory_manager
{
public:
    typedef memory_manager* pointer_t;

    static pointer_t getInstance() throw(...);
    const HANDLE getThreadHeap();

    template <typename T> T mAlloc(_In_ size_t nAllocationSize)
    {
        return mAlloc<T>(GetCurrentThreadId(), nAllocationSize);
    }

    template <typename T> T mAllocChars(_In_ size_t nAllocationSizeInChars)
    {
        return mAlloc<T>(GetCurrentThreadId(), nAllocationSizeInChars * sizeof(TCHAR));
    }

    template <typename T> T* mAllocTypeUnit()
    {
        return mAllocTypeUnit<T>(GetCurrentThreadId());
    }

    template <typename T> T reAlloc(_In_ T pMemory, _In_ size_t nAllocationSize);

    template <typename T> void free(__drv_freesMem(Mem) _Frees_ptr_ T pMemory)
    {
        free(GetCurrentThreadId(), pMemory);
    }

    template <typename T> T mAlloc(_In_ DWORD nThreadId, _In_ size_t nAllocationSize);

    template <typename T> T* mAllocTypeUnit(_In_ DWORD nThreadId)
    {
        return mAlloc<T*>(nThreadId, sizeof(T));
    }

    template <typename T> void free(_In_ DWORD nThreadId, __drv_freesMem(Mem) _Frees_ptr_ T pMemory);


private:
    typedef std::unique_ptr<memory_manager> unique_ptr_t;
    typedef std::unordered_map<DWORD, std::unique_ptr<private_heap>> heaps_map_type;

    memory_manager() noexcept
    {
    }

    static unique_ptr_t m_instance;
    static WAPI srw_lock m_instanceLock;

    heaps_map_type m_privateHeaps;
    // The 'm_heapsLock' synchronizes access to the 'm_privateHeaps'.
    WAPI srw_lock m_heapsLock;
};
