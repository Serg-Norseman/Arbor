#pragma once
#include "dlllayer\arborvis.h"
#include "service\com\impl.h"

/**
 * `arbor_visual_impl` class.
 * Only impements the `IArborVisual`.
 */
class arbor_visual_impl: public ATLADD implements<IArborVisual>
{
public:
    virtual HRESULT STDMETHODCALLTYPE createWindow(_Out_ HWND* handle) override;


private:
    const HWND m_hwnd = nullptr;
};
