#pragma once
#include "dlllayer\arborvis.h"
#include "graph\edge.h"
#include "graph\vertex.h"
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
    virtual HRESULT STDMETHODCALLTYPE clear() override;
    virtual HRESULT STDMETHODCALLTYPE addEdge(
        _In_ std::wstring&& tail, _In_ std::wstring&& head, _In_ float length) override;
    virtual HRESULT STDMETHODCALLTYPE addEdge(
        _In_ ARBOR vertex* tail,
        _In_ ARBOR vertex* head,
        _In_ const float length,
        _In_ const float stiffness,
        _In_ const bool directed,
        _In_ const D2D1_COLOR_F& color,
        _Outptr_result_maybenull_ ARBOR edge** e);
    virtual HRESULT STDMETHODCALLTYPE addEdge(
        _In_ ARBOR vertex* tail,
        _In_ ARBOR vertex* head,
        _In_ const float length,
        _In_ const bool directed,
        _In_ const D2D1_COLOR_F& color,
        _Outptr_result_maybenull_ ARBOR edge** e);
    virtual HRESULT STDMETHODCALLTYPE addVertex(
        _In_ std::wstring&& name,
        _In_ const D2D1_COLOR_F& bkgndColor,
        _In_ const D2D1_COLOR_F& textColor,
        _In_ float mass,
        _In_ bool fixed,
        _Outptr_result_maybenull_ ARBOR vertex** v);


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
