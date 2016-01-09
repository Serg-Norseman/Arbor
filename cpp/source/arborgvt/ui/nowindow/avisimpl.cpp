#include "ui\nowindow\avisimpl.h"

/**
 * Creates a target window object where graph is rendered.
 *
 * Parameters:
 * >handle
 * Pointer to a variable receiving target HWND.
 *
 * Returns:
 * Standard HRESULT code.
 */
HRESULT arbor_visual_impl::createWindow(_Out_ HWND* handle)
{
    *handle = m_hwnd;
    return E_NOTIMPL;
}
