#pragma once
#include <dwrite.h>
#include <wincrypt.h>

#pragma region "standard" C/Windows types
/*
 * <void*> argument.
 */
template void* memory_manager::mAlloc<void*>(_In_ size_t nAllocationSize);
template void memory_manager::free<void*>(_In_ void* pMemory);
/*
 * <LPTSTR> argument.
 */
template LPTSTR memory_manager::mAlloc<LPTSTR>(_In_ size_t nAllocationSize);
template void memory_manager::free<LPTSTR>(_In_z_ LPTSTR pMemory);
#pragma endregion instantiations for "standard" C/Windows types

#pragma region types from the ATLADD namespace
#pragma endregion instantiations for types from the ATLADD namespace

#pragma region types from the OLEDB namespace
#pragma endregion instantiations for types from the OLEDB namespace
