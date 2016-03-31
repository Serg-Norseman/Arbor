#pragma once
#include "graph\edge.h"
#include "graph\vertex.h"
#include "service\com\comptr.h"
#include "service\sse.h"
#include "service\stladdon.h"
#include <d2d1.h>
#include <d2d1_1.h>
#include <dwrite.h>

class element_draw
{
public:
    void __vectorcall createDeviceResources(_In_ ID2D1DeviceContext* deviceContext, _In_ const __m128 color)
    {
        sse_t value;
        _mm_store_ps(value.data, color);
        deviceContext->CreateSolidColorBrush(
            reinterpret_cast<D2D1_COLOR_F*> (value.data), nullptr, m_brush.getAddressOf());
    }

    void releaseDeviceResources()
    {
        m_brush.reset();
    }

    // Returns brush to draw element background.
    HRESULT getBrush(_COM_Outptr_result_maybenull_ ID2D1SolidColorBrush** brush) const
    {
        if (m_brush)
        {
            m_brush.copyTo(brush);
            return S_OK;
        }
        else
        {
            *brush = nullptr;
            return S_FALSE;
        }
    }


protected:
    ATLADD com_ptr<ID2D1SolidColorBrush> m_brush;
};

/**
 * `edge_draw` and `vertex_draw` draw their parent objects -- a vertex and an edge respectively.
 */
class edge_draw: public element_draw
{
public:
    void createDeviceResources(_In_ ID2D1DeviceContext* deviceContext, _In_ const ARBOR edge& object)
    {
        base_class_t::createDeviceResources(deviceContext, object.getColor());
    }


private:
    typedef element_draw base_class_t;
};

class vertex_draw: public element_draw
{
public:
    explicit vertex_draw(_In_ ATLADD com_ptr<IDWriteTextLayout>&& textLayout)
        :
        m_textBrush {},
        m_textLayout {std::move(textLayout)}
    {
    }

    void createDeviceResources(_In_ ID2D1DeviceContext* deviceContext, _In_ const ARBOR vertex& object)
    {
        base_class_t::createDeviceResources(deviceContext, object.getColor());
        sse_t value;
        _mm_store_ps(value.data, object.getTextColor());
        deviceContext->CreateSolidColorBrush(
            reinterpret_cast<D2D1_COLOR_F*> (value.data), nullptr, m_textBrush.getAddressOf());
    }

    void releaseDeviceResources()
    {
        m_textBrush.reset();
        base_class_t::releaseDeviceResources();
    }

    HRESULT getTextLayout(_COM_Outptr_result_maybenull_ IDWriteTextLayout** layout) const
    {
        if (m_textLayout)
        {
            m_textLayout.copyTo(layout);
            return S_OK;
        }
        else
        {
            *layout = nullptr;
            return S_FALSE;
        }
    }

    HRESULT getTextBrush(_COM_Outptr_result_maybenull_ ID2D1SolidColorBrush** brush) const
    {
        if (m_textBrush)
        {
            m_textBrush.copyTo(brush);
            return S_OK;
        }
        else
        {
            *brush = nullptr;
            return S_FALSE;
        }
    }

ATLPREFAST_SUPPRESS(4701)
    HRESULT __vectorcall getArea(_In_ const __m128 center, _Out_ D2D1_ELLIPSE* area) const
    {
        DWRITE_TEXT_METRICS metrics;
        HRESULT hr = m_textLayout ? m_textLayout->GetMetrics(&metrics) : E_POINTER;
        if (SUCCEEDED(hr))
        {
            sse_t value;
            _mm_store_ps(value.data, center);
            // Add some space around the text (read: use 0.75 as the multiplier and not 0.5).
            *area = D2D1::Ellipse(
                D2D1::Point2F(value.data[0], value.data[1]), metrics.width * 0.75f, metrics.height * 0.75f);
        }
        return hr;
    }
ATLPREFAST_UNSUPPRESS()


private:
    typedef element_draw base_class_t;

    ATLADD com_ptr<ID2D1SolidColorBrush> m_textBrush;
    ATLADD com_ptr<IDWriteTextLayout> m_textLayout;
};
