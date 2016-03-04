#include "service\memorymg.h"
#include "service\memmgti.h"
#include "service\stladdon.h"

memory_manager::unique_ptr_t memory_manager::m_instance;
WAPI srw_lock memory_manager::m_instanceLock;

// The initial size of the process heap, in bytes. This value will be rounded up to the next page boundary.
#define HEAP_INITIAL_SIZE (4096)

#pragma region private_heap
/**
 * private_heap ctor.
 * Creates private heap object that can be used by the calling thread.
 *
 * Parameters:
 * >nHeapInitialSize
 * The initial size of the heap, in bytes. This value determines the initial amount of memory that is committed for the
 * heap. The value will be rounded up to the next page boundary.
 */
private_heap::private_heap(_In_ size_t nHeapInitialSize)
{
    SYSTEM_INFO systemInfo;
    BOOL bWOW64Process = FALSE;
    if (IsWow64Process(GetCurrentProcess(), &bWOW64Process) && bWOW64Process)
    {
        GetNativeSystemInfo(&systemInfo);
    }
    else
    {
        GetSystemInfo(&systemInfo);
    }

    size_t nMod = nHeapInitialSize % systemInfo.dwPageSize;
    if (nMod)
    {
        nHeapInitialSize = nHeapInitialSize - nMod + systemInfo.dwPageSize;
    }

    m_heap.reset(HeapCreate(HEAP_NO_SERIALIZE, nHeapInitialSize, 0));
    if (m_heap &&
        (bWOW64Process || (PROCESSOR_ARCHITECTURE_INTEL == systemInfo.wProcessorArchitecture)))
    {
        // More at the http://blogs.msdn.com/michael_howard/archive/2008/02/18/
        // faq-about-heapsetinformation-in-windows-vista-and-heap-based-buffer-overruns.aspx
        HeapSetInformation(m_heap.get(), HeapEnableTerminationOnCorruption, nullptr, 0);
    }
}
#pragma endregion private_heap implementation



#pragma region memory_manager
/**
 * Returns pointer to one and only one memory_manager object instance, used in the application. It also allocates if
 * necessary per-thread private heap object (for Debug and Release builds). No other members do this.
 *
 * Parameters:
 * None.
 *
 * Returns:
 * Pointer to the memory_manager instance.
 *
 * Throws:
 * >std::bad_alloc
 * When instantiation of the singleton failed.
 */
memory_manager::pointer_t memory_manager::getInstance() noexcept(false)
{
    bool bInstantiated = false;
    // Make a scope for the shared lock.
    {
        STLADD lock_guard_shared<WAPI srw_lock> lock {m_instanceLock};
        bInstantiated = static_cast<bool> (m_instance);
    }
    if (!bInstantiated)
    {
        STLADD lock_guard_exclusive<WAPI srw_lock> lock {m_instanceLock};
        if (!m_instance)
        {
            m_instance.reset(new memory_manager {});
        }
    }
    // The shared lock's scope is the entire method 'cos 'm_instance' is used through the method.
    STLADD lock_guard_shared<WAPI srw_lock> lock {m_instanceLock};
    if (m_instance)
    {
        // Create private heap object on per-thread basis.
        DWORD nCurrentThreadId = GetCurrentThreadId();
        STLADD lock_guard_exclusive<WAPI srw_lock> heapsLock {m_instance->m_heapsLock};
        if (m_instance->m_privateHeaps.cend() == m_instance->m_privateHeaps.find(nCurrentThreadId))
        {
            auto heap = std::make_unique<private_heap>(HEAP_INITIAL_SIZE);
            if (heap && heap->getHeap())
            {
                m_instance->m_privateHeaps.emplace(nCurrentThreadId, std::move(heap));
            }
        }
    }
    else
    {
        throw std::bad_alloc {};
    }

    return m_instance.get();
}


/**
 * Returns handle to a heap object used by the current thread.
 *
 * Parameters:
 * None.
 *
 * Returns:
 * Handle to the heap object.
 */
const HANDLE memory_manager::getThreadHeap()
{
    STLADD lock_guard_shared<WAPI srw_lock> heapsLock {m_heapsLock};
    heaps_map_type::const_iterator it = m_privateHeaps.find(GetCurrentThreadId());
    return (m_privateHeaps.cend() != it) ? it->second->getHeap() : nullptr;
}


/**
 * Reallocates a block of memory in an application heap. The allocated memory is not movable.
 * Heap to use is selected for the current thread (by its id).
 *
 * Parameters:
 * >pMemory
 * A pointer to the block of memory that the function reallocates. This pointer is returned by an earlier call to the
 * mAlloc or reAlloc methods.
 *
 * Returns:
 * Pointer to the reallocated memory block.
 */
template <typename T>
T memory_manager::reAlloc(_In_ T pMemory, _In_ size_t nAllocationSize)
{
    T p = nullptr;
    HANDLE hHeap = nullptr;
    {
        STLADD lock_guard_shared<WAPI srw_lock> heapsLock {m_heapsLock};
        heaps_map_type::const_iterator it = m_privateHeaps.find(GetCurrentThreadId());
        hHeap = (m_privateHeaps.cend() != it) ? it->second->getHeap() : nullptr;
    }
    if (nullptr != hHeap)
    {
        p = static_cast<T> (HeapReAlloc(hHeap, 0, pMemory, nAllocationSize));
    }
    return p;
}


/**
 * Allocates a block of memory from an application heap. The allocated memory is not movable.
 * The "T" typename is a pointer-to-type.
 *
 * Parameters:
 * >nThreadId
 * Identifier of a thread. This thread must create a private heap object before call this method. That heap will be used
 * to allocate memory block there.
 * >nAllocationSize
 * The number of bytes to be allocated.
 *
 * Returns:
 * Pointer to the allocated memory block.
 */
template <typename T>
T memory_manager::mAlloc(_In_ DWORD nThreadId, _In_ size_t nAllocationSize)
{
    T p = nullptr;
    HANDLE hHeap = nullptr;
    {
        STLADD lock_guard_shared<WAPI srw_lock> heapsLock {m_heapsLock};
        heaps_map_type::const_iterator it = m_privateHeaps.find(nThreadId);
        hHeap = (m_privateHeaps.cend() != it) ? it->second->getHeap() : nullptr;
    }
    if (nullptr != hHeap)
    {
        p = static_cast<T> (HeapAlloc(hHeap, 0, nAllocationSize));
    }
    return p;
}


/**
 * Frees a memory block allocated in an application heap by the mAlloc or reAlloc methods.
 * Heap to use is selected for the current thread (by its id).
 *
 * Parameters:
 * >nThreadId
 * Identifier of a thread. This thread must create a private heap object before call this method. That heap will be used
 * to free the specified memory block there.
 * >pMemory
 * A pointer to the memory block to be freed.
 *
 * Returns:
 * N/A.
 */
template <typename T>
void memory_manager::free(_In_ DWORD nThreadId, __drv_freesMem(Mem) _Frees_ptr_ T pMemory)
{
    if (pMemory)
    {
        HANDLE hHeap = nullptr;
        {
            STLADD lock_guard_shared<WAPI srw_lock> heapsLock {m_heapsLock};
            heaps_map_type::const_iterator it = m_privateHeaps.find(nThreadId);
            hHeap = (m_privateHeaps.cend() != it) ? it->second->getHeap() : nullptr;
        }
        if (nullptr != hHeap)
        {
            HeapFree(hHeap, 0, pMemory);
        }
    }
}
#pragma endregion memory_manager implementation
