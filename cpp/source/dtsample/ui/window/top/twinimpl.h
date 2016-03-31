#pragma once
#include "ns\atladd.h"
#include "service\com\comptr.h"
#include "service\winapi\icon.h"
#include "service\winapi\menu.h"
#include "ui\window\wi.h"

ATLADD_BEGIN
class top_window_impl abstract: public window_impl<top_window_impl>
{
public:
    virtual BOOL ProcessWindowMessage(
        _In_ HWND hWnd,
        _In_ UINT nMessage,
        _In_ WPARAM nWParam,
        _In_ LPARAM nLParam,
        _Inout_ LRESULT& nLResult,
        _In_ DWORD nMsgMapID) override;


protected:
    virtual LRESULT createHandler() override;

    virtual void destroyHandler() override
    {
        storeWindowPlacement();
        // Release COM objects before call to 'CoUninitialize' will be made.
        releaseAllFactories();
    }

    bool loadWindowPlacement(_In_ const WINDOWPLACEMENT* pData);
    void storeWindowPlacement(_Out_ WINDOWPLACEMENT* pData) const;


private:
    typedef window_impl<top_window_impl> base_class_t;

    virtual bool loadWindowPlacement() = 0;
    virtual void storeWindowPlacement() const = 0;

    WAPI icon_t m_smallIcon;
    WAPI icon_t m_defaultIcon;
    WAPI menu_t m_menu;
};
ATLADD_END
