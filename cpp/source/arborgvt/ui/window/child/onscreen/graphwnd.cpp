#include "service\sse.h"
#include "ui\window\child\onscreen\graphwnd.h"
#include <random>

ATLADD_BEGIN

/**
 * Creates this window.
 *
 * Parameters:
 * >hParent
 * Handle to the parent window.
 *
 * Returns:
 * Handle to this window.
 */
_Check_return_ HWND graph_window::create(_In_ const HWND hParent)
{
    typedef ATL::CWinTraits<WS_CHILD | WS_CLIPSIBLINGS | WS_TABSTOP | WS_VISIBLE, WS_EX_NOPARENTNOTIFY> style_traits_t;
    return create(hParent, style_traits_t::GetWndStyle(0), style_traits_t::GetWndExStyle(0));
}


/**
 * Creates this window.
 *
 * Parameters:
 * >parent
 * Handle to the parent window.
 * >style
 * The style of the window being created.
 * >exStyle
 * The extended window style of the window being created.
 *
 * Returns:
 * Handle to this window.
 */
_Check_return_ HWND graph_window::create(_In_opt_ const HWND parent, _In_ DWORD style, _In_ DWORD exStyle)
{
    return Create(parent, ATL::_U_RECT {nullptr}, nullptr, style, exStyle, 0U, nullptr);
}


/**
 * Handler of ATL message map.
 *
 * Parameters:
 * >hWnd
 * Handle of this window.
 * >nMessage
 * Message to process.
 * >nWParam
 * Additional message information. The contents of this parameter depend on the value of the nMessage parameter.
 * >nLParam
 * Additional message information. The contents of this parameter depend on the value of the nMessage parameter.
 * >nLResult
 * The result of the message processing and depends on the message sent (nMessage).
 * >nMsgMapID
 * The identifier of the message map that will process the message.
  *
 * Returns:
 * Non-zero value if the message was processed, or zero otherwise.
*/
BOOL graph_window::ProcessWindowMessage(
    _In_ HWND hWnd,
    _In_ UINT nMessage,
    _In_ WPARAM nWParam,
    _In_ LPARAM nLParam,
    _Inout_ LRESULT& nLResult,
    _In_ DWORD nMsgMapID)
{
    BOOL bHandled;
    if (m_dpiChangedMessage == nMessage)
    {
        base_class_t::ProcessWindowMessage(hWnd, WM_DPICHANGED, nWParam, nLParam, nLResult, nMsgMapID);
        bHandled = TRUE;
    }
    else
    {
        switch (nMessage)
        {
            case WM_HSCROLL:
            {
                scrollHandler(SB_HORZ, LOWORD(nWParam), HIWORD(nWParam));
                nLResult = 0;
                bHandled = TRUE;
            }
            break;

            case WM_VSCROLL:
            {
                scrollHandler(SB_VERT, LOWORD(nWParam), HIWORD(nWParam));
                nLResult = 0;
                bHandled = TRUE;
            }
            break;

            default:
            {
                bHandled = base_class_t::ProcessWindowMessage(hWnd, nMessage, nWParam, nLParam, nLResult, nMsgMapID);
            }
        }
    }

    return bHandled;
}


/**
 * WM_CREATE message handler.
 *
 * Parameters:
 * None.
 *
 * Returns:
 * To continue creation of the window returns 0. To destroy the window returns -1.
 */
LRESULT graph_window::createHandler()
{
    LRESULT nResult = base_class_t::createHandler();
    if (!nResult)
    {
        D2D1_STROKE_STYLE_PROPERTIES1 prop = D2D1::StrokeStyleProperties1(
            D2D1_CAP_STYLE_FLAT,
            D2D1_CAP_STYLE_FLAT,
            D2D1_CAP_STYLE_FLAT,
            D2D1_LINE_JOIN_MITER,
            10.0f,
            D2D1_DASH_STYLE_DASH,
            0.0f,
            D2D1_STROKE_TRANSFORM_TYPE_NORMAL);
        m_direct2DFactory->CreateStrokeStyle(prop, nullptr, 0, m_areaStrokeStyle.getAddressOf());
#if defined(_DEBUG) || defined(SHOW_FPS)
        createTextFormatForBodyText(m_framesPerSecondTextFormat.getAddressOf());
#endif
    }
    return nResult;
}


/**
 * Renders this window.
 *
 * Parameters:
 * None.
 *
 * Returns:
 * N/A.
 */
void graph_window::draw()
{
#if defined(_DEBUG) || defined(SHOW_FPS)
    std::chrono::high_resolution_clock clock {};
    std::chrono::high_resolution_clock::time_point first = clock.now();
#endif

    D2D1_SIZE_F targetSize = m_direct2DContext->GetSize();
    // Here XMM's the first and the second zeros can be omitted.
    sse_t value = {targetSize.width, targetSize.height, 0.0f, 0.0f};
    __m128 size = _mm_load_ps(value.data);
    __m128 viewBound = m_graph.getViewBound();

    m_direct2DContext->Clear(D2D1::ColorF {GetSysColor(COLOR_WINDOW), 1.0f});
    /*
     * Draw vertices and edges.
     *
     * To draw a vertex I need to get its coordinate. To draw an edge I need coordinates of both tail vertex and head
     * vertex.
     *
     * Drawing vertices and edges in two separate loops I end up with double calculation of coordinate of each vertex
     * (one calculation for a vertex itself and and another one -- for edge points). That's evil #1.
     *
     * I can use single loop that draws an edge and its vertices on each iteration. Here I need to guarantee that I draw
     * each vertex only once (for example, if I draw a vertex with transparency effect and so on). Therefore I must have
     * a lookup table where already drawn vertices are stored. And before draw a vertex I check that table. That's evil
     * #2.
     *
     * I didn't compare the evils. But I believe that searching in a lookup table ain't faster than double calculation
     * made by SSE instructions. Especially, when size of the graph (and therefore size of the lookup table) will be big
     * enough.
     *
     * This method may change graph's objects (it sets user-defined data for vertices and edges). Therefore this method
     * has to obtain _exclusive_ locks before it can access vertices and/or edges. Thus no one can change graph while
     * this method renders it. This method must obtain vertices lock first and then edges lock -- only this order is
     * allowed; otherwise a deadlock may occur.
     */
    {
        // Begin scopes for locks (they exploit RAII).
        STLADD lock_guard_exclusive<WAPI srw_lock> verticesLock {m_graph.getVerticesLock(), std::try_to_lock};
        if (verticesLock)
        {
            // I see no sense to draw only vertices (under a designated lock) on the first step and then draw edges
            // having locks on the both containers.
            STLADD lock_guard_exclusive<WAPI srw_lock> edgesLock {m_graph.getEdgesLock(), std::try_to_lock};
            if (edgesLock)
            {
                for (auto it = m_graph.verticesBegin(); m_graph.verticesEnd() != it; ++it)
                {
                    __m128 coordinate = it->getCoordinates();
                    if (0b0011 == (0b0011 & _mm_movemask_ps(_mm_cmpeq_ps(coordinate, coordinate))))
                    {
                        coordinate = graphToLogical(coordinate, size, viewBound);
                        auto draw = static_cast<vertex_draw*> (it->getData());
                        if (!draw)
                        {
                            /*
                             * If a vertex was added after device resources had created...
                             *
                             * This method may modify `m_graph`'s elements; that's why `draw` is non-const method.
                             * I don't want to issue additional loops on each `WM_PAINT` before a render stage -- this
                             * is a waste of CPU resources.
                             */
                            ATLADD com_ptr<IDWriteTextLayout> layout {};
                            if (SUCCEEDED(createTextLayout(it->getName(), layout.getAddressOf())))
                            {
                                draw = new vertex_draw {std::move(layout)};
                                m_vertices.emplace_back(draw);
                                it->setData(draw);
                                draw->createDeviceResources(m_direct2DContext.get(), *it);
                            }
                        }
                        D2D1_ELLIPSE area;
                        if (SUCCEEDED(draw->getArea(coordinate, &area)))
                        {
                            /*
                             * Be aware that `vertex_draw::getXXXXBrush` method below uses COM reference counting that
                             * can be omitted here, 'cos `brush` is definitely local-only COM object.
                             *
                             * If you can guarantee that 'out' parameter of `vertex_draw::getXXXXBrush` method is always
                             * local only, you can safely modify `getBrush` in a way that it will not use COM reference
                             * counting.
                             */
                            ATLADD com_ptr<ID2D1SolidColorBrush> brush {};
                            if (S_OK == draw->getBrush(brush.getAddressOf()))
                            {
                                m_direct2DContext->FillEllipse(area, brush.get());
                            }
                            brush.reset();
                            if (S_OK == draw->getTextBrush(brush.getAddressOf()))
                            {
                                if (m_areaStrokeStyle)
                                {
                                    m_direct2DContext->DrawEllipse(area, brush.get(), 1.0f, m_areaStrokeStyle.get());
                                }
                                ATLADD com_ptr<IDWriteTextLayout> layout {};
                                if (S_OK == draw->getTextLayout(layout.getAddressOf()))
                                {
                                    DWRITE_TEXT_METRICS metrics;
                                    if (SUCCEEDED(layout->GetMetrics(&metrics)))
                                    {
                                        D2D1_POINT_2F origin = D2D1::Point2F(
                                            area.point.x - (metrics.width * 0.5f),
                                            area.point.y - (metrics.height * 0.5f));
                                        m_direct2DContext->DrawTextLayout(
                                            origin, layout.get(), brush.get(), D2D1_DRAW_TEXT_OPTIONS_NONE);
                                    }
                                }
                            }
                        }
                    }
                }
                for (auto it = m_graph.edgesBegin(); m_graph.edgesEnd() != it; ++it)
                {
                    const ARBOR vertex* tail = (*it)->getTail();
                    __m128 tailCoordinate = tail->getCoordinates();
                    if (0b0011 == (0b0011 & _mm_movemask_ps(_mm_cmpeq_ps(tailCoordinate, tailCoordinate))))
                    {
                        const ARBOR vertex* head = (*it)->getHead();
                        __m128 headCoordinate = head->getCoordinates();
                        if (0b0011 == (0b0011 & _mm_movemask_ps(_mm_cmpeq_ps(headCoordinate, headCoordinate))))
                        {
                            tailCoordinate = graphToLogical(tailCoordinate, size, viewBound);
                            headCoordinate = graphToLogical(headCoordinate, size, viewBound);
                            // Get tail and head ellipses.
                            auto tailDraw = static_cast<vertex_draw*> (tail->getData());
                            auto headDraw = static_cast<vertex_draw*> (head->getData());
                            D2D1_ELLIPSE tailArea;
                            D2D1_ELLIPSE headArea;
                            if (SUCCEEDED(tailDraw->getArea(tailCoordinate, &tailArea)) &&
                                SUCCEEDED(headDraw->getArea(headCoordinate, &headArea)))
                            {
                                auto draw = static_cast<edge_draw*> ((*it)->getData());
                                if (!draw)
                                {
                                    draw = new edge_draw {};
                                    m_edges.emplace_back(draw);
                                    (*it)->setData(draw);
                                    draw->createDeviceResources(m_direct2DContext.get(), **it);
                                }
                                ATLADD com_ptr<ID2D1SolidColorBrush> brush {};
                                if (S_OK == draw->getBrush(brush.getAddressOf()))
                                {
                                    connectAreas(tailArea, headArea, (*it)->getDirected(), brush.get());
                                }
                            }
                        }
                    }
                }
            }
        }
    }

#if defined(_DEBUG) || defined(SHOW_FPS)
    /*
     * Calculate and show average FPS.
     */
    std::chrono::high_resolution_clock::duration duration = getAverageFrameTime(clock.now() - first);
    auto durationInMicroseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration);
    auto durationInSeconds = std::chrono::duration_cast<std::chrono::duration<double>>(duration);
    double fps = 1.0 / durationInSeconds.count();
    int length = _sctprintf(TEXT("%.4f FPS (%I64i µs per frame)"), fps, durationInMicroseconds.count()) + 1;
    STLADD t_char_unique_ptr_t text {new TCHAR[length]};
    length =
        _stprintf_s(text.get(), length, TEXT("%.4f FPS (%I64i µs per frame)"), fps, durationInMicroseconds.count());
    m_direct2DContext->DrawText(
        text.get(),
        length,
        m_framesPerSecondTextFormat.get(),
        D2D1::RectF(0.0f, 0.0f, targetSize.width, targetSize.height),
        m_framesPerSecondBrush.get());
#endif
    if (m_graph.active())
    {
        Invalidate(FALSE);
    }
}


/*
 * Initializes D2D device specific resources.
 *
 * Parameters:
 * None.
 *
 * Returns:
 * N/A.
 *
 * Remarks:
 * This method may create device-independent resources (for example, `IDWriteTextLayout` objects) bound to a vertex or
 * to an edge. But the method does it only once, if an object doesn't have an initialized device-independent resource.
 */
void graph_window::createDeviceResources()
{
    for (auto it = m_graph.verticesBegin(); m_graph.verticesEnd() != it; ++it)
    {
        auto draw = static_cast<vertex_draw*> (it->getData());
        if (!draw)
        {
            ATLADD com_ptr<IDWriteTextLayout> layout {};
            if (SUCCEEDED(createTextLayout(it->getName(), layout.getAddressOf())))
            {
                draw = new vertex_draw {std::move(layout)};
                m_vertices.emplace_back(draw);
                it->setData(draw);
            }
        }
        draw->createDeviceResources(m_direct2DContext.get(), *it);
    }
    for (auto it = m_graph.edgesBegin(); m_graph.edgesEnd() != it; ++it)
    {
        auto draw = static_cast<edge_draw*> ((*it)->getData());
        if (!draw)
        {
            draw = new edge_draw {};
            m_edges.emplace_back(draw);
            (*it)->setData(draw);
        }
        draw->createDeviceResources(m_direct2DContext.get(), **it);
    }
#if defined(_DEBUG) || defined(SHOW_FPS)
    m_direct2DContext->CreateSolidColorBrush(
        D2D1::ColorF {D2D1::ColorF::Red, 1.0f}, m_framesPerSecondBrush.getAddressOf());
#endif
}


/**
 * Releases previously allocated D2D device specific resources.
 *
 * Parameters:
 * None.
 *
 * Returns:
 * N/A.
 */
void graph_window::releaseDeviceResources()
{
#if defined(_DEBUG) || defined(SHOW_FPS)
    m_framesPerSecondBrush.reset();
#endif
    std::for_each(
        m_vertices.begin(),
        m_vertices.end(),
        [] (_In_ auto& value) -> void
        {
            value->releaseDeviceResources();
        });
    std::for_each(
        m_edges.begin(),
        m_edges.end(),
        [] (_In_ auto& value) -> void
        {
            value->releaseDeviceResources();
        });
}


/**
 * Transforms a point in the Direct2D render target's coordinate space (logical coordinates of the D2D device context)
 * to the graph coordinate space.
 *
 * Parameters:
 * >value
 * Logical coordinates of the local D2D context to be converted to the graph's space. Must not be NaN; the method
 * doesn't check this.
 * >logicalSize
 * Size of the Direct2D device context render surface.
 * >viewBound
 * Size of the graph's view area. See Remarks section for more details.
 *
 * Returns:
 * `value` coordinates converted to the graph coordinate space.
 *
 * Remarks:
 * The graph's view area is a way to create initial animation of all vertices in the graph. When graph animation begins
 * its view area grows from `4x4' predefined size upto "graph size" (area occupied by all vertices on the current
 * animation step). To exploit this initial animation this method requires size of "view area", not "graph area".
 *
 * Because of some features of the algorithm that calculates vertices position (Barnes Hut and something else I still
 * ain't aware; it's required more time to dig into C# legacy code) if the method makes direct mapping between the graph
 * coordinate space and logical space of the Direct2D context, some vertices go beyond the bounds of the HWND. That's
 * why I have to use the margins (as C# code does).
 *
 * In current implementation this method uses `HSUBPS` instruction from SSE3 set without checking CPU capabilities.
 */
__m128 graph_window::logicalToGraph(_In_ const __m128 value, _In_ const __m128 logicalSize, _In_ const __m128 viewBound)
{
    sse_t marginMem = {m_margin, m_margin, 0.0f, 0.0f};
    __m128 margin = _mm_load_ps(marginMem.data);
    __m128 temp = _mm_sub_ps(logicalSize, margin);
    __m128 viewSize = _mm_hsub_ps(viewBound, viewBound);
    viewSize = _mm_div_ps(viewSize, temp);
    marginMem.data[0] = 0.5f;
    marginMem.data[1] = 0.5f;
    temp = _mm_load_ps(marginMem.data);
    margin = _mm_mul_ps(margin, temp);
    temp = _mm_sub_ps(value, margin);
    temp = _mm_mul_ps(temp, viewSize);
    viewSize = _mm_shuffle_ps(viewBound, value, 0b11101101);
    return _mm_add_ps(temp, viewSize);
}


/**
 * Transforms a point in the graph coordinate space to Direct2D render target's coordinate space (logical coordinates
 * of the D2D device context).
 *
 * Parameters:
 * >value
 * Coordinates in the graph's space to be converted to the logical coordinates of the local D2D context. Must not be
 * NaN; the method doesn't check this.
 * >logicalSize
 * Size of the Direct2D device context render surface.
 * >viewBound
 * Size of the graph's view area. See Remarks section for more details.
 *
 * Returns:
 * `value` coordinates converted to logical coordinates of the Direct2D context.
 *
 * Remarks:
 * The graph's view area is a way to create initial animation of all vertices in the graph. When graph animation begins
 * its view area grows from `4x4' predefined size upto "graph size" (area occupied by all vertices on the current
 * animation step). To exploit this initial animation this method requires size of "view area", not "graph area".
 *
 * Because of some features of the algorithm that calculates vertices position (Barnes Hut and something else I still
 * ain't aware; it's required more time to dig into C# legacy code) if the method makes direct mapping between the graph
 * coordinate space and logical space of the Direct2D context, some vertices go beyond the bounds of the HWND. That's
 * why I have to use the margins (as C# code does).
 *
 * In current implementation this method uses `HSUBPS` instruction from SSE3 set without checking CPU capabilities.
 */
__m128 graph_window::graphToLogical(_In_ const __m128 value, _In_ const __m128 logicalSize, _In_ const __m128 viewBound)
{
    // Here XMM's the first and the second zeros can be omitted.
    sse_t marginMem = {m_margin, m_margin, 0.0f, 0.0f};
    __m128 margin = _mm_load_ps(marginMem.data);
    __m128 temp = _mm_sub_ps(logicalSize, margin);
    __m128 viewSize = _mm_hsub_ps(viewBound, viewBound);
    viewSize = _mm_div_ps(temp, viewSize);
    temp = _mm_shuffle_ps(viewBound, value, 0b11101101);
    temp = _mm_sub_ps(value, temp);
    temp = _mm_mul_ps(temp, viewSize);
    marginMem.data[0] = 0.5f;
    marginMem.data[1] = 0.5f;
    viewSize = _mm_load_ps(marginMem.data);
    margin = _mm_mul_ps(margin, viewSize);
    return _mm_add_ps(temp, margin);
}


/**
 * WM_HSCROLL/WM_VSCROLL messages handler.
 *
 * Parameters:
 * >nBar
 * Specifies the type of scroll bar (SB_HORZ or SB_VERT).
 * >nScrollingRequest
 * User's scrolling request.
 * >nPosition
 * The current position of the scroll box if the 'nScrollingRequest' is SB_THUMBPOSITION or SB_THUMBTRACK.
 *
 * Returns:
 * N/A.
 */
void graph_window::scrollHandler(_In_ int nBar, _In_ const WORD nScrollingRequest, _In_ const WORD nPosition)
{
    SCROLLINFO si;
    si.cbSize = sizeof(si);
    si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
    ::GetScrollInfo(::GetParent(m_hWnd), nBar, &si);
    int nPrevPos = si.nPos;
    switch (nScrollingRequest)
    {
        case SB_TOP:
        {
            si.nPos = si.nMin;
        }
        break;

        case SB_BOTTOM:
        {
            si.nPos = si.nMax;
        }
        break;

        case SB_LINEUP:
        {
            // 1 is _logical_ pixel here...
            si.nPos -= 1;
        }
        break;

        case SB_LINEDOWN:
        {
            // ...and here
            si.nPos += 1;
        }
        break;

        case SB_PAGEUP:
        {
            si.nPos -= si.nPage;
        }
        break;

        case SB_PAGEDOWN:
        {
            si.nPos += si.nPage;
        }
        break;

        case SB_THUMBTRACK:
        {
            si.nPos = nPosition;
        }
        break;

        default:
        {
        }
        break;
    }

    // Set the position and then retrieve it. Due to adjustments by Windows it may not be the same as the value
    // set.
    si.fMask = SIF_POS;
    ::SetScrollInfo(::GetParent(m_hWnd), nBar, &si, TRUE);
    ::GetScrollInfo(::GetParent(m_hWnd), nBar, &si);

    // If the position has changed, scroll the window.
    if (nPrevPos != si.nPos)
    {
        scrollContent(nBar, si.nPos);
    }
}


/**
 * Scrolls this window content.
 *
 * Parameters:
 * >nBar
 * Specifies the type of scroll bar (SB_HORZ or SB_VERT).
 * >nPos
 * Scrolling position (physical value).
 *
 * Returns:
 * N/A.
 */
void graph_window::scrollContent(_In_ int nBar, _In_ const int nPos)
{
    if (m_direct2DContext)
    {
        float fDPIX;
        float fDPIY;
        if (SUCCEEDED(getDpiForMonitor(m_hWnd, &fDPIX, &fDPIY)))
        {
            D2D1_MATRIX_3X2_F transform;
            m_direct2DContext->GetTransform(&transform);
            D2D1_SIZE_F offset {};
            if (SB_HORZ == nBar)
            {
                offset = D2D1::SizeF(physicalToLogical(-nPos, fDPIX), transform._32);
            }
            else if (SB_VERT == nBar)
            {
                offset = D2D1::SizeF(transform._31, physicalToLogical(-nPos, fDPIY));
            }
            m_direct2DContext->SetTransform(D2D1::Matrix3x2F::Translation(offset));
            Invalidate(FALSE);
            UpdateWindow();
        }
    }
}


#pragma region text
/**
 * Creates DirectWrite's IDWriteTextLayout object that represents the specified text.
 *
 * Parameters:
 * >text
 * Source text.
 * >textLayout
 * Pointer to pointer that receives the result object (if the method succeeded in terms of the HRESULT).
 *
 * Returns:
 * Standard HRESULT code.
 */
HRESULT graph_window::createTextLayout(
    _In_ const STLADD string_type* text, _COM_Outptr_result_maybenull_ IDWriteTextLayout** textLayout) const
{
    D2D1_SIZE_F size;
    if (m_direct2DContext)
    {
        size = m_direct2DContext->GetSize();
    }
    else
    {
        RECT rect;
        GetClientRect(&rect);
        float fDPIX;
        float fDPIY;
        if (SUCCEEDED(getDpiForMonitor(m_hWnd, &fDPIX, &fDPIY)))
        {
            size.width = physicalToLogical(rect.right, fDPIX);
            size.height = physicalToLogical(rect.bottom, fDPIY);
        }
        else
        {
            size.width = static_cast<float> (rect.right);
            size.height = static_cast<float> (rect.bottom);
        }
    }
    size.width *= m_vertexNameWidth;
    ATLADD com_ptr<IDWriteTextLayout> layout {};
    HRESULT hr = base_class_t::createTextLayoutForBodyTitle(text->c_str(), text->size(), &size, layout.getAddressOf());
    if (SUCCEEDED(hr))
    {
        hr = layout->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
        if (SUCCEEDED(hr))
        {
            ATLADD com_ptr<IDWriteInlineObject> sign {};
            hr = m_directWriteFactory->CreateEllipsisTrimmingSign(layout.get(), sign.getAddressOf());
            if (SUCCEEDED(hr))
            {
                DWRITE_TRIMMING trimmingOptions {DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0};
                hr = layout->SetTrimming(&trimmingOptions, sign.get());
            }
        }
    }
    *textLayout = SUCCEEDED(hr) ? layout.detach() : nullptr;
    return hr;
}
#pragma endregion text related methods definitions


/**
 * Draws new edge between two specified vertices.
 *
 * Parameters:
 * >tailArea
 * Tail vertex.
 * >headVertex
 * Head vertex.
 * >directed
 * Determines whether an edge must be drawn as directed one.
 * >brush
 * A brush to draw the edge on the Direct2D device context.
 *
 * Returns:
 * N/A.
 *
 * Remarks:
 * Current implement can't connect a vertex with itself (it doesn't draw a closed edge).
 */
void graph_window::connectAreas(
    _In_ const D2D1_ELLIPSE& tailArea,
    _In_ const D2D1_ELLIPSE& headArea,
    _In_ const bool directed,
    _In_ ID2D1SolidColorBrush* brush) const noexcept
{
    D2D1_POINT_2F tailPoint;
    if (getEllipsePoint(tailArea, headArea, &tailPoint))
    {
        D2D1_POINT_2F headPoint;
        if (getEllipsePoint(headArea, tailArea, &headPoint))
        {
            /*
             * Check the vectors co-direction.
             */
            if (((0 < (tailArea.point.x - headArea.point.x)) && (0 < (tailPoint.x - headPoint.x))) ||
                ((0 > (tailArea.point.x - headArea.point.x)) && (0 > (tailPoint.x - headPoint.x))) ||
                ((0 < (tailArea.point.y - headArea.point.y)) && (0 < (tailPoint.y - headPoint.y))) ||
                ((0 > (tailArea.point.y - headArea.point.y)) && (0 > (tailPoint.y - headPoint.y))))
            {
                m_direct2DContext->DrawLine(tailPoint, headPoint, brush);
                if (directed)
                {
                    D2D1_POINT_2F left;
                    D2D1_POINT_2F right;
                    if (getArrow(tailPoint, headPoint, &left, &right))
                    {
                        m_direct2DContext->DrawLine(left, headPoint, brush);
                        m_direct2DContext->DrawLine(right, headPoint, brush);
                    }
                }
            }
        }
    }
}


/**
 * Finds out an intersection point of the first specififed ellipse and the segment that begins at the center point of
 * one ellipse and ends at the center of another ellipse.
 *
 * Parameters:
 * >tailArea
 * The first ellipse (defines the curve that intersects the segment).
 * >headVertex
 * The second ellipse.
 * >point
 * The intersection point. When the center point of the `headVertex` is outside of the area occupied by the `tailArea`
 * this intersection point is always defined. Otherwise the `point` is undefined.
 *
 * Returns:
 * `true` value if the `point` is defined and `false` otherwise.
 *
 * Remarks:
 * This method solves the following task:
 * Let's assume we have two ellipses (`tailArea` and `headArea`). The center point of the first ellipse is (x0, y0); the
 * center point of the second ellipse is (x1, y1). Ellipse equation of the first ellipse is:
 *  (x - x0) * (x - x0)     (y - y0) * (y - y0)
 * --------------------- + --------------------- = 1,
 *         a * a                   b * b
 * where 'a' and 'b' is the x-radius and y-radius of the ellipse respectively.
 * We also have the following line:
 *  x - x0     y - y0
 * -------- = --------,
 *    m          n
 * where vector l(m, n) is the directing vector of the line that crosses the center points of the both ellipses.
 * Therefore m = x1 - x0 and n = y1 - y0.
 *
 * If we solve the above system of equations we get the answer:
 *                        b
 * x = x0 ± ------------------------- ('+' when x1 > x0 and '-' when x1 < x0),
 *                 n * n     b * b
 *           sqrt(------- + -------)
 *                 m * m     a * a
 *           n
 * y = y0 + --- * (x - x0),
 *           m
 * where x is from [x0, x1] and y is from [y0, y1].
 *
 * In current implementation this method uses `HSUBPS`, `HADDPS` and `ADDSUBPS` instructions from SSE3 set without
 * checking CPU capabilities.
 */
_Success_(return) bool graph_window::getEllipsePoint(
    _In_ const D2D1_ELLIPSE& tailArea, _In_ const D2D1_ELLIPSE& headArea, _Out_ D2D1_POINT_2F* point) const noexcept
{
    /*
     * Check position of the `headArea.point`. For `point` be valid, `headArea.point` must be outside of the area of the
     * `tailArea`.
     */
    bool defined = true;
    if ((tailArea.point.x - tailArea.radiusX <= headArea.point.x) &&
        (tailArea.point.x + tailArea.radiusX >= headArea.point.x))
    {
        sse_t value = {headArea.point.x - tailArea.point.x, tailArea.radiusX, 1.0f, 0.0f};
        __m128 temp = _mm_load_ps(value.data);
        temp = _mm_mul_ps(temp, temp);
        __m128 temp2 = _mm_shuffle_ps(temp, temp, 0b11100101);
        temp = _mm_div_ss(temp, temp2);
        temp = _mm_shuffle_ps(temp, temp, 0b11100010);
        temp = _mm_hsub_ps(temp, temp);
        temp = _mm_sqrt_ss(temp);
        value = {tailArea.radiusY, tailArea.point.y, 0.0f, 0.0f};
        temp2 = _mm_load_ps(value.data);
        temp = _mm_mul_ss(temp, temp2);
        temp2 = _mm_shuffle_ps(temp2, temp2, 0b11100101);
        temp = _mm_shuffle_ps(temp, temp, 0b11100000);
        temp = _mm_addsub_ps(temp2, temp);
        /*
         * More longer sample, I believe:
         *
         * value.data[0] = headArea.point.y;
         * temp2 = _mm_load_ps(value.data);
         * temp2 = _mm_shuffle_ps(temp2, temp2, 0);
         * defined =
         *     (0b0001 & _mm_movemask_ps(_mm_cmplt_ps(temp2, temp))) ||
         *     (0b0010 & _mm_movemask_ps(_mm_cmpgt_ps(temp2, temp)));
         *
         * This sample can replace the two lines below.
         */
        _mm_store_ps(value.data, temp);
        defined = (value.data[0] > headArea.point.y) || (value.data[1] < headArea.point.y);
    }
    if (defined)
    {
        /*
         * `headArea.point` is outside of `tailArea` ellipse. Get the intersection point.
         */
        __m128 xy;
        if (tailArea.point.x != headArea.point.x)
        {
            sse_t value = {headArea.point.x, headArea.point.y, tailArea.radiusX, tailArea.radiusY};
            __m128 temp = _mm_load_ps(value.data);
            value = {tailArea.point.x, tailArea.point.y, 0.0f, 0.0f};
            __m128 temp2 = _mm_load_ps(value.data);
            temp = _mm_sub_ps(temp, temp2);
            temp = _mm_mul_ps(temp, temp);
            temp2 = _mm_shuffle_ps(temp, temp, 0b11110101);
            temp = _mm_div_ps(temp2, temp);
            temp = _mm_shuffle_ps(temp, temp, 0b11011000);
            temp = _mm_hadd_ps(temp, temp);
            temp = _mm_rsqrt_ss(temp);
            value.data[0] = tailArea.radiusY;
            value.data[1] = tailArea.point.x;
            temp2 = _mm_load_ps(value.data);
            temp = _mm_mul_ss(temp2, temp);
            temp2 = _mm_shuffle_ps(temp2, temp2, 0b11100101);
            xy = (tailArea.point.x < headArea.point.x) ? _mm_add_ss(temp2, temp) : _mm_sub_ss(temp2, temp);
        }
        else
        {
            sse_t value;
            value.data[0] = tailArea.point.x;
            xy = _mm_load_ps(value.data);
        }
        // Get 'y' coordinate value from computed 'x'.
        sse_t value;
        value = {tailArea.point.x, tailArea.radiusX, 1.0f, 0.0f};
        __m128 temp = _mm_load_ps(value.data);
        __m128 temp2 = _mm_shuffle_ps(xy, temp, 0b01100100);
        temp2 = _mm_shuffle_ps(temp2, temp, 0b11101100);
        temp = _mm_sub_ss(temp2, temp);
        temp = _mm_mul_ps(temp, temp);
        temp2 = _mm_shuffle_ps(temp, temp, 0b11100101);
        temp = _mm_div_ss(temp, temp2);
        temp = _mm_shuffle_ps(temp, temp, 0b11100010);
        temp = _mm_hsub_ps(temp, temp);
        temp2 = _mm_shuffle_ps(temp2, temp2, 0b11100111);
        if (0x01 & _mm_movemask_ps(_mm_cmplt_ss(temp, temp2)))
        {
            __m128 temp3 = _mm_shuffle_ps(temp2, temp, 0b01100100);
            temp = _mm_shuffle_ps(temp3, temp, 0b11101100);
        }
        temp = _mm_sqrt_ss(temp);
        value = {tailArea.radiusY, tailArea.point.y, 0.0f, 0.0f};
        temp2 = _mm_load_ps(value.data);
        temp = _mm_mul_ss(temp, temp2);
        temp2 = _mm_shuffle_ps(temp2, temp2, 0b11100101);
        temp = (tailArea.point.y < headArea.point.y) ? _mm_add_ss(temp2, temp) : _mm_sub_ss(temp2, temp);
        xy = _mm_shuffle_ps(xy, temp, 0b00000000);
        _mm_store_ps(value.data, xy);
        *point = D2D1::Point2F(value.data[0], value.data[2]);
    }
    return defined;
}


/**
 * Draws an arrow head at the specified point.
 *
 * Parameters:
 * >tailPoint
 * Beginning of a directed edge (arrow).
 * >headPoint
 * Head point of the arrow.
 * >left
 * Arrow head point #1.
 * >right
 * Arrow head point #2.
 *
 * Returns:
 * `true` value if the caller should draw the arrow and `false` otherwise.
 *
 * Remarks:
 * In current implementation this method uses `HSUBPS`, `HADDPS` and `ADDSUBPS` instructions from SSE3 set without
 * checking CPU capabilities.
 */
_Success_(return) bool graph_window::getArrow(
    _In_ const D2D1_POINT_2F& tailPoint,
    _In_ const D2D1_POINT_2F& headPoint,
    _Out_ D2D1_POINT_2F* left,
    _Out_ D2D1_POINT_2F* right) const noexcept
{
    // `DPPS` is part of SSE4.1.
    sse_t value = {headPoint.x, tailPoint.x, headPoint.y, tailPoint.y};
    __m128 temp = _mm_load_ps(value.data);
    __m128 source = _mm_hsub_ps(temp, temp);
    if (simd_cpu_capabilities::sse41())
    {
        temp = _mm_dp_ps(source, source, 0b00111111);
    }
    else
    {
        temp = _mm_shuffle_ps(source, source, 0b01000100);
        temp = _mm_mul_ps(temp, temp);
        temp = _mm_hadd_ps(temp, temp);
    }
    temp = _mm_sqrt_ps(temp);
    value = {m_arrowLength, m_arrowHalfWidth, 0.0f, 0.0f};
    __m128 length = _mm_load_ps(value.data);
    bool result = 0x01 & _mm_movemask_ps(_mm_cmplt_ps(length, temp));
    if (result)
    {
        __m128 tan = _mm_div_ps(_mm_shuffle_ps(source, source, 0b11110101), source);
        tan = _mm_shuffle_ps(tan, tan, 0b11101000);
        temp = _mm_mul_ss(tan, tan);
        value = {1.0f, headPoint.x, headPoint.y, 1.0f};
        __m128 temp2 = _mm_load_ps(value.data);
        temp = _mm_add_ss(temp, temp2);
        temp = _mm_rsqrt_ss(temp);
        temp = _mm_mul_ss(length, temp);
        // Prepare `temp` and `temp2` for the following `ADDSUBPS`.
        temp = _mm_shuffle_ps(temp, temp, 0b11100000);
        temp2 = _mm_shuffle_ps(temp2, temp2, 0b11100101);
        __m128 a = _mm_addsub_ps(temp2, temp);
        if (tailPoint.x > headPoint.x)
        {
            a = _mm_shuffle_ps(a, a, 0b11100101);
        }
        temp = _mm_sub_ss(a, temp2);
        temp = _mm_mul_ss(tan, temp);
        temp2 = _mm_shuffle_ps(temp2, temp2, 0b11100110);
        temp = _mm_add_ss(temp, temp2);
        a = _mm_shuffle_ps(a, temp, 0);
        a = _mm_shuffle_ps(a, a, 0b11101000);

        temp = _mm_rcp_ss(_mm_mul_ss(tan, tan));
        temp2 = _mm_shuffle_ps(temp2, temp2, 0b11100111);
        temp = _mm_add_ss(temp, temp2);
        temp = _mm_rsqrt_ss(temp);
        temp2 = _mm_shuffle_ps(length, temp2, 0b11100101);
        temp = _mm_mul_ss(temp, temp2);
        temp = _mm_shuffle_ps(temp, temp, 0b11100000);
        temp2 = _mm_shuffle_ps(a, a, 0b01010000);
        __m128 b = _mm_addsub_ps(temp2, temp);
        temp = _mm_sub_ps(b, temp2);
        temp2 = _mm_rcp_ps(tan);
        temp = _mm_mul_ps(temp, temp2);
        temp2 = _mm_shuffle_ps(a, a, 0b11100101);
        temp = _mm_add_ps(temp, temp2);
        b = _mm_shuffle_ps(b, temp, 0b01000001);
        _mm_store_ps(value.data, b);
        *left = D2D1::Point2F(value.data[0], value.data[2]);
        *right = D2D1::Point2F(value.data[1], value.data[3]);
    }
    return result;
}


#if defined(_DEBUG) || defined(SHOW_FPS)
/**
 * Calculates average time spent to render a frame by this window.
 * This method stores time of some last frames and calculates an average value through that frames.
 *
 * Parameters:
 * >frameTime
 * Time spent to render the last frame.
 *
 * Returns:
 * Average time, this window spends to render a frame.
 *
 * Remarks:
 * Unit of measurement is `std::chrono::high_resolution_clock::period` of second.
 */
std::chrono::high_resolution_clock::duration graph_window::getAverageFrameTime(
    _In_ std::chrono::high_resolution_clock::duration&& frameTime)
{
    std::chrono::high_resolution_clock::duration::rep frame = frameTime.count();
    m_frameTotal -= *m_frameIt;
    m_frameTotal += frame;
    *m_frameIt = frame;
    auto result = std::chrono::high_resolution_clock::duration {m_frameTotal / m_frameTimes.size()};
    if (m_framesMaxNumber > m_frameTimes.size())
    {
        m_frameTimes.push_back(0);
        m_frameIt = m_frameTimes.end() - 1;
    }
    else
    {
        ++m_frameIt;
        if (m_frameTimes.end() == m_frameIt)
        {
            m_frameIt = m_frameTimes.begin();
        }
    }
    return result;
}
#endif

ATLADD_END
