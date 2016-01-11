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
    virtual LRESULT createHandler() override;
    virtual void sizeHandler(_In_ const LONG nNewWidth, _In_ const LONG nNewHeight) override;
    virtual void draw() const override;

    virtual void createDeviceResources() override
    {
        m_direct2DContext->CreateSolidColorBrush(
            D2D1::ColorF {GetSysColor(COLOR_WINDOWTEXT), 1.0f}, m_brush.getAddressOf());
        m_direct2DContext->CreateSolidColorBrush(m_color, m_ellipseBrush.getAddressOf());
    }

    virtual void releaseDeviceResources() override
    {
        m_ellipseBrush.reset();
        m_brush.reset();
    }


private:
    typedef child_window_impl base_class_t;

    void scrollHandler(_In_ int nBar, _In_ const WORD nScrollingRequest, _In_ const WORD nPosition);
    void scrollContent(_In_ int nBar, _In_ const int nPos);

    UINT m_dpiChangedMessage;
    D2D1_COLOR_F m_color {D2D1::ColorF {GetSysColor(COLOR_WINDOWTEXT), 1.0f}};
    ATLADD com_ptr<ID2D1SolidColorBrush> m_brush {};
    ATLADD com_ptr<ID2D1SolidColorBrush> m_ellipseBrush {};
    ATLADD com_ptr<ID2D1PathGeometry1> m_g {};
    ATLADD com_ptr<ID2D1EllipseGeometry> m_ellipse {};
    ATLADD com_ptr<IDWriteTextFormat> m_tf {};
};
ATLADD_END
