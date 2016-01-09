#pragma once
#include "ns\atladd.h"
#include "service\com\comptr.h"
#include "service\stladdon.h"
#include "service\winapi\chkerror.h"
#include "ui\window\top\twinimpl.h"
#include <dwrite_2.h>
#include <ShlObj.h>
#include <vector>
#include <wincodec.h>

ATLADD_BEGIN
/**
 * ATLADD desktop_sample_window
 * Main top window of the application.
 */
class desktop_sample_window final: public top_window_impl
{
public:
    typedef std::vector<HANDLE> handles_type;

    explicit desktop_sample_window(_In_ const UINT nTaskbarButtonCreatedMessage)
        :
        m_taskbarList3 {},
        m_hAppStartingCursor {nullptr},
        m_nTaskbarButtonCreatedMessage {nTaskbarButtonCreatedMessage},
        m_threads {}
    {
        WAPI check_hr(m_imagingFactory.coCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER));
    }

    _Check_return_ virtual HWND create(_In_ const HWND hParent) override;

    _Check_return_ bool preTranslateMessage(_In_ MSG* pMsg)
    {
        return IsWindow() && IsDialogMessage(pMsg);
    }

    virtual BOOL ProcessWindowMessage(
        _In_ HWND hWnd,
        _In_ UINT nMessage,
        _In_ WPARAM nWParam,
        _In_ LPARAM nLParam,
        _Inout_ LRESULT& nLResult,
        _In_ DWORD nMsgMapID) override;

    const handles_type* getThreadsToWaitFor() const
    {
        return &m_threads;
    }

    void updateWindowText(_In_opt_ const STLADD string_type* pszText);
    void handleThreadTermination(_In_ const HANDLE hObject);


protected:
    virtual LRESULT createHandler() override;
    virtual void destroyHandler() override;

    virtual void draw() override
    {
        m_direct2DContext->Clear(D2D1::ColorF {GetSysColor(COLOR_WINDOW), 1.0f});
    }


private:
    typedef top_window_impl base_class_t;

    virtual bool loadWindowPlacement() override
    {
        return false;
    }

    virtual void storeWindowPlacement() const override
    {
    }

    _Check_return_ bool commandHandler(_In_ const UINT nId);
    
    BOOL isAppStartingCursorMustBeSet() const
    {
        return (
            m_threads.size() &&
            (WAIT_TIMEOUT == WaitForMultipleObjects(static_cast<DWORD> (m_threads.size()), m_threads.data(), TRUE, 0)));
    }

    ATLADD com_ptr<ITaskbarList3> m_taskbarList3;
    HCURSOR m_hAppStartingCursor;
    const UINT m_nTaskbarButtonCreatedMessage;

    handles_type m_threads;
};
ATLADD_END
