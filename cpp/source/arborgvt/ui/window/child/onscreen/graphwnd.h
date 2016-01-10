#pragma once
#include "ui\window\child\cwi.h"

ATLADD_BEGIN
class graph_window: public child_window_impl<graph_window>
{
public:
    explicit graph_window(_In_ UINT dpiChangedMessage)
        :
        base_class_t(true),
        m_dpiChangedMessage {dpiChangedMessage}
    {
    }

    _Check_return_ virtual HWND create(_In_ const HWND hParent) override;
    _Check_return_ virtual HWND create(_In_ const HWND parent, _In_ DWORD style, _In_ DWORD exStyle);
    virtual BOOL ProcessWindowMessage(
        _In_ HWND hWnd,
        _In_ UINT nMessage,
        _In_ WPARAM nWParam,
        _In_ LPARAM nLParam,
        _Inout_ LRESULT& nLResult,
        _In_ DWORD nMsgMapID) override;


protected:
    virtual void draw() override;


private:
    typedef child_window_impl base_class_t;

    UINT m_dpiChangedMessage;
};
ATLADD_END
