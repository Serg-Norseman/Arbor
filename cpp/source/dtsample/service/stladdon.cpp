#include "service\stladdon.h"

// The initial size of the process heap, in bytes. This value will be rounded up to the next page boundary.
const size_t HeapInitialSize = 4096;

STLADD_BEGIN

WAPI heap_t private_heap::m_heap;

/**
 * private_heap default ctor.
 * Creates the single private heap object that can be used by this library. Initial size of the heap is
 * `HeapInitialSize`. This value determines the initial amount of memory that is committed for the heap. The value will
 * be rounded up to the next page boundary.
 *
 * Parameters:
 * None.
 *
 * Returns:
 * Handle to the private heap object, used by a thread, attached this DLL, and by threads, spawned by this DLL.
 */
HANDLE private_heap::createHeap()
{
    HANDLE heap;
    SYSTEM_INFO systemInfo;
    BOOL wow64Process = FALSE;
    if (IsWow64Process(GetCurrentProcess(), &wow64Process) && wow64Process)
    {
        GetNativeSystemInfo(&systemInfo);
    }
    else
    {
        GetSystemInfo(&systemInfo);
    }

    size_t heapInitialSize = HeapInitialSize;
    size_t mod = HeapInitialSize % systemInfo.dwPageSize;
    if (mod)
    {
        heapInitialSize = HeapInitialSize - mod + systemInfo.dwPageSize;
    }
    // The heap must be protected from multiple access by different threads. Because it will be used by a thread, that
    // attached the DLL, and a GUI thread, the will be created by the DLL.
    heap = HeapCreate(0, heapInitialSize, 0);
    if (heap && (wow64Process || (PROCESSOR_ARCHITECTURE_INTEL == systemInfo.wProcessorArchitecture)))
    {
        HeapSetInformation(heap, HeapEnableTerminationOnCorruption, nullptr, 0);
    }
    return heap;
}
#pragma endregion private_heap implementation

STLADD_END
