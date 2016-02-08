#pragma once
#include "dlllayer\arborvis.h"
#include "service\com\impl.h"
#include "ui\window\child\onscreen\graphwnd.h"
#include <memory>
#include <thread>

/**
 * `arbor_visual_impl` class.
 * Only impements the `IArborVisual`.
 */
class arbor_visual_impl: public ATLADD implements<IArborVisual>
{
public:
    ~arbor_visual_impl()
    {
        m_thread.join();
    }

    virtual HRESULT STDMETHODCALLTYPE createWindow(
        _In_opt_ HWND parent,
        _In_ DWORD style,
        _In_ DWORD exStyle,
        _In_ HANDLE hwndReadyEvent,
        _In_ UINT dpiChangedMessage) override;
    virtual HRESULT STDMETHODCALLTYPE getHWND(_Out_ HWND* handle) override;
    virtual HRESULT STDMETHODCALLTYPE addEdge(
        _In_ std::wstring&& tail, _In_ std::wstring&& head, _In_ float length) override;


private:
    void createWindowImpl(
        _In_opt_ HWND parent,
        _In_ DWORD style,
        _In_ DWORD exStyle,
        _In_ HANDLE hwndReadyEvent,
        _In_ UINT dpiChangedMessage);

    std::thread m_thread;
    std::unique_ptr<ATLADD graph_window> m_window;
};
