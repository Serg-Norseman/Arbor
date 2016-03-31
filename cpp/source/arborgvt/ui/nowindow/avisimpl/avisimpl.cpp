#include "ui\nowindow\avisimpl\avisimpl.h"

/**
 * Creates a target window object where graph is rendered. The window is created asynchronously.
 *
 * Parameters:
 * >parent
 * A handle to the parent or owner window of the window being created.
 * >style
 * The style of the window being created.
 * >exStyle
 * The extended window style of the window being created.
 * >hwndReadyEvent
 * Handle to an event that is set to signaled state after the new window got valid HWND.
 * >dpiChangedMessage
 * A message sent to the new window when `parent` window gets `WM_DPICHANGED` message.
 *
 * Returns:
 * Standard HRESULT code.
 *
 * Remarks:
 * The new window created on a dedicated thread. After the thread has created HWND it sends `message` to the `parent`
 * window.
 * The method creates only one window. If the window was already created the method sends `message` immediately.
 * For some reason `WM_DPICHANGED` message can't cross thread bounds. Therefore the `parent` window sends
 * `dpiChangedMessage` message instead of "direct" `WM_DPICHANGED`.
 */
HRESULT arbor_visual_impl::createWindow(
    _In_opt_ HWND parent, _In_ DWORD style, _In_ DWORD exStyle, _In_ HANDLE hwndReadyEvent, _In_ UINT dpiChangedMessage)
{
    /*
     * In order to make the code more platform-independent I use `std::thread` from C++ library and not MSFT Windows
     * specific `CreateThread`. Even if I can lost some performance.
     */
    m_thread = std::thread
        {&arbor_visual_impl::createWindowImpl, this, parent, style, exStyle, hwndReadyEvent, dpiChangedMessage};
    return m_thread.joinable() ? S_OK : E_FAIL;
}


/**
 * Gets handle to the window where graph is rendered.
 *
 * Parameters:
 * >handle
 * Pointer to variable that receives handle to the window. If the window doesn't have valid HWND the parameter receives
 * `nullptr` and the method returns `S_FALSE`.
 *
 * Returns:
 * Standard HRESULT code.
 */
HRESULT arbor_visual_impl::getHWND(_Out_ HWND* handle)
{
    *handle = m_window ? m_window->m_hWnd : nullptr;
    return *handle ? S_OK : S_FALSE;
}


/**
 * Removes all edges and vertices from the underlying graph.
 *
 * Parameters:
 * None.
 *
 * Returns:
 * Standard HRESULT code.
 */
HRESULT arbor_visual_impl::clear()
{
    if (m_window)
    {
        m_window->clear();
        return S_OK;
    }
    else
    {
        return E_POINTER;
    }
}


/**
 * Adds a new edge to graph. The new edge connects two specified vertices.
 *
 * Parameters:
 * >tail
 * Name of the tail vertex, where the new edge begins.
 * >head
 * Name of the head vertex, where the new edge ends.
 * >length
 * Size of the new edge.
 *
 * Returns:
 * Standard HRESULT code.
 *
 * Remarks:
 * The following problem exists with type of the `tail` and `head` parameters: it must be a type independent from
 * this library code and caller code. If it will be, for example, `stladd::arbor::string_type`, a caller has to
 * implement `stladd::arbor::private_heap::createHeap` method, duplicating implementation made by the DLL. If type of
 * the parameters will be defined by a caller code... that's a stupid idea at all.
 *
 * `std::wstring` was selected as the type as implicitly the closest type.
 *
 * This implementation doesn't invalidate the underlying HWND.
 */
HRESULT arbor_visual_impl::addEdge(_In_ std::wstring&& tail, _In_ std::wstring&& head, _In_ float length)
{
    if (m_window)
    {
        m_window->addEdge({tail.cbegin(), tail.cend()}, {head.cbegin(), head.cend()}, length);
        return S_OK;
    }
    else
    {
        return E_POINTER;
    }
}


/**
 * Adds a new edge to graph. The new edge connects two specified vertices.
 *
 * Parameters:
 * >tail
 * Tail vertex, where the new edge begins.
 * >head
 * Head vertex, where the new edge ends.
 * >length
 * Size of the new edge.
 * >stiffness
 * New edge stiffness.
 * >directed
 * Determines edge style: is it directed or not.
 * >color
 * Edge drawing color.
 * >e
 * Result edge.
 *
 * Returns:
 * Standard HRESULT code.
 *
 * Remarks:
 * This implementation doesn't invalidate the underlying HWND.
 */
HRESULT arbor_visual_impl::addEdge(
    _In_ ARBOR vertex* tail,
    _In_ ARBOR vertex* head,
    _In_ const float length,
    _In_ const float stiffness,
    _In_ const bool directed,
    _In_ const D2D1_COLOR_F& color,
    _Outptr_result_maybenull_ ARBOR edge** e)
{
    if (m_window)
    {
        *e = m_window->addEdge(tail, head, length, stiffness, directed, color);
        return S_OK;
    }
    else
    {
        *e = nullptr;
        return E_POINTER;
    }
}


/**
 * Adds a new edge to graph. The new edge connects two specified vertices.
 *
 * Parameters:
 * >tail
 * Tail vertex, where the new edge begins.
 * >head
 * Head vertex, where the new edge ends.
 * >length
 * Size of the new edge.
 * >directed
 * Determines edge style: is it directed or not.
 * >color
 * Edge drawing color.
 * >e
 * Result edge.
 *
 * Returns:
 * Standard HRESULT code.
 *
 * Remarks:
 * This implementation doesn't invalidate the underlying HWND.
 */
HRESULT arbor_visual_impl::addEdge(
    _In_ ARBOR vertex* tail,
    _In_ ARBOR vertex* head,
    _In_ const float length,
    _In_ const bool directed,
    _In_ const D2D1_COLOR_F& color,
    _Outptr_result_maybenull_ ARBOR edge** e)
{
    if (m_window)
    {
        *e = m_window->addEdge(tail, head, length, directed, color);
        return S_OK;
    }
    else
    {
        *e = nullptr;
        return E_POINTER;
    }
}


/**
 * Adds a new vertex to the graph if the latter doesn't have a vertex with the same name.
 *
 * Parameters:
 * >name
 * New vertex name.
 * >bkgndColor
 * Vertex background color.
 * >textColor
 * Vertex text color.
 * >mass
 * Vertex mass.
 * >fixed
 * Vertex movement ability.
 * >v
 * Result vertex;
 *
 * Returns:
 * Standard HRESULT code.
 *
 * Remarks:
 * See Remarks section for the `arbor_visual_impl::addEdge` method.
 */
HRESULT arbor_visual_impl::addVertex(
    _In_ std::wstring&& name,
    _In_ const D2D1_COLOR_F& bkgndColor,
    _In_ const D2D1_COLOR_F& textColor,
    _In_ float mass,
    _In_ bool fixed,
    _Outptr_result_maybenull_ ARBOR vertex** v)
{
    if (m_window)
    {
        *v = m_window->addVertex({name.cbegin(), name.cend()}, bkgndColor, textColor, mass, fixed);
        return S_OK;
    }
    else
    {
        *v = nullptr;
        return E_POINTER;
    }
}


/**
 * Creates a new window. The method is executing on a dedicated thread.
 *
 * Parameters:
 * >parent
 * A handle to the parent or owner window of the window being created.
 * >style
 * The style of the window being created.
 * >exStyle
 * The extended window style of the window being created.
 * >hwndReadyEvent
 * Handle to an event that is set to signaled state after the new window got valid HWND.
 * >dpiChangedMessage
 * A message sent to the new window when `parent` window gets `WM_DPICHANGED` message.
 *
 * Returns:
 * N/A.
 */
void arbor_visual_impl::createWindowImpl(
    _In_opt_ HWND parent, _In_ DWORD style, _In_ DWORD exStyle, _In_ HANDLE hwndReadyEvent, _In_ UINT dpiChangedMessage)
{
    // WAM requires COINIT_APARTMENTTHREADED (see Hilo sample documentation)! Using 'COINIT_MULTITHREADED' can end up
    // with unpredictable behaviour.
    if (SUCCEEDED(::CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
    {
        m_window = std::make_unique<ATLADD graph_window>(dpiChangedMessage);
        if (m_window)
        {
            HWND hwnd = m_window->create(parent, style, exStyle);
            if (nullptr != hwnd)
            {
                SetEvent(hwndReadyEvent);
                MSG msg;
                while (0 < GetMessage(&msg, nullptr, 0, 0))
                {
                    ::TranslateMessage(&msg);
                    ::DispatchMessage(&msg);
                }

                /*
                 * Destroy window explicitly (if it wasn't already) before window C++ object will be deleted -- this is
                 * requirement of the ATL (see destructor of the ATL::CWindowImplRoot<T> for the Debug configuration).
                 */
                if (m_window->IsWindow())
                {
                    m_window->DestroyWindow();
                }
            }
            // Release window C++ object before call to the 'CoUninitialize'.
            m_window.reset();
        }
        ::CoUninitialize();
    }
}
