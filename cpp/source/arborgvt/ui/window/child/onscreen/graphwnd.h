#pragma once
#include "graph/graph.h"
#include "service/com/comptr.h"
#include "service/stladdon.h"
#include "ui/nowindow/graph/draw.h"
#include "ui/window/child/cwi.h"
#if defined(_DEBUG) || defined(SHOW_FPS)
#include <chrono>
#endif

ATLADD_BEGIN

/**
 * A HWND object that renders the graph.
 *
 * The class contains `new` and `delete` overloaded operators to avoid 'C4316' warning when build x86 release (object
 * allocated on the heap may not be aligned 16). The class has a data member of `ARBOR graph` type that must be aligned
 * on a 16-byte boundary.
 */
class graph_window: public child_window_impl<graph_window>
{
public:
    explicit graph_window(_In_ UINT dpiChangedMessage)
        :
        base_class_t(true),
        m_graph {},
        m_vertices {},
        m_edges {},
        m_areaStrokeStyle {},
        m_dpiChangedMessage {dpiChangedMessage}
#if defined(_DEBUG) || defined(SHOW_FPS)
        ,
        m_frameTimes {0},
        m_frameIt {m_frameTimes.begin()},
        m_frameTotal {0},
        m_framesPerSecondTextFormat {},
        m_framesPerSecondBrush {}
#endif
    {
    }

    static void* operator new(_In_ const size_t size)
    {
        STLADD aligned_sse_allocator<graph_window> allocator {};
        return allocator.allocate(size, 0);
    }

    static void operator delete(_In_ void* p)
    {
        STLADD aligned_sse_allocator<graph_window> allocator {};
        allocator.deallocate(p);
    }

    _Check_return_ virtual HWND create(_In_ const HWND parent) override;
    _Check_return_ virtual HWND create(_In_opt_ const HWND parent, _In_ DWORD style, _In_ DWORD exStyle);
    virtual BOOL ProcessWindowMessage(
        _In_ HWND hwnd,
        _In_ UINT message,
        _In_ WPARAM wParam,
        _In_ LPARAM lParam,
        _Inout_ LRESULT& lResult,
        _In_ DWORD msgMapID) override;

    ARBOR edge* addEdge(
        _In_ ARBOR vertex* tail,
        _In_ ARBOR vertex* head,
        _In_ const float length,
        _In_ const float stiffness,
        _In_ const bool directed,
        _In_ const D2D1_COLOR_F& color)
    {
        return m_graph.addEdge(tail, head, length, stiffness, directed, color);
    }

    ARBOR edge* addEdge(
        _In_ ARBOR vertex* tail,
        _In_ ARBOR vertex* head,
        _In_ const float length,
        _In_ const bool directed,
        _In_ const D2D1_COLOR_F& color)
    {
        return m_graph.addEdge(tail, head, length, directed, color);
    }

    void addEdge(_In_ STLADD string_type&& tail, _In_ STLADD string_type&& head, _In_ float length)
    {
        m_graph.addEdge(std::move(tail), std::move(head), length);
    }

    ARBOR vertex* addVertex(
        _In_ STLADD string_type&& name,
        _In_ const D2D1_COLOR_F& bkgndColor,
        _In_ const D2D1_COLOR_F& textColor,
        _In_ float mass,
        _In_ bool fixed)
    {
        return m_graph.addVertex(std::move(name), bkgndColor, textColor, mass, fixed);
    }

    void clear() noexcept
    {
        m_graph.clear();
        m_vertices.clear();
        m_edges.clear();
    }


protected:
    virtual LRESULT createHandler() override;
    virtual void draw() override;

    virtual void createDeviceResources() override;
    virtual void releaseDeviceResources() override;


private:
    typedef child_window_impl<graph_window> base_class_t;
    typedef std::vector<std::unique_ptr<vertex_draw>, STLADD default_allocator<std::unique_ptr<vertex_draw>>>
        vertices_draw_cont_t;
    typedef std::vector<std::unique_ptr<edge_draw>, STLADD default_allocator<std::unique_ptr<edge_draw>>>
        edges_draw_cont_t;

    static __m128 __vectorcall logicalToGraph(
        _In_ const __m128 value, _In_ const __m128 logicalSize, _In_ const __m128 viewBound);
    static __m128 __vectorcall graphToLogical(
        _In_ const __m128 value, _In_ const __m128 logicalSize, _In_ const __m128 viewBound);

    void scrollHandler(_In_ int bar, _In_ const WORD scrollingRequest, _In_ const WORD position);
    void scrollContent(_In_ int bar, _In_ const int pos);
    HRESULT createTextLayout(
        _In_ const STLADD string_type* text, _COM_Outptr_result_maybenull_ IDWriteTextLayout** textLayout) const;
    void __fastcall connectAreas(
        _In_ const D2D1_ELLIPSE& tailArea,
        _In_ const D2D1_ELLIPSE& headArea,
        _In_ const bool directed,
        _In_ ID2D1SolidColorBrush* brush) const noexcept;
    _Success_(return) bool __fastcall getEllipsePoint(
        _In_ const D2D1_ELLIPSE& tailArea,
        _In_ const D2D1_ELLIPSE& headArea,
        _Out_ D2D1_POINT_2F* point) const noexcept;
    _Success_(return) bool __fastcall getArrow(
        _In_ const D2D1_POINT_2F& tailPoint,
        _In_ const D2D1_POINT_2F& headPoint,
        _Out_ D2D1_POINT_2F* left,
        _Out_ D2D1_POINT_2F* right) const noexcept;
#if defined(_DEBUG) || defined(SHOW_FPS)
    std::chrono::high_resolution_clock::duration getAverageFrameTime(
        _In_ std::chrono::high_resolution_clock::duration&& frameTime);
#endif

    static constexpr D2D1_SIZE_F m_vertexNameSize = {50.0f, 50.0f};
    static constexpr float m_arrowLength = 7.75f;
    static constexpr float m_arrowHalfWidth = 1.5f;
    // Value of the `m_margin` depends on size of a vertex area.
    static constexpr float m_margin = 100.0f;

    ARBOR graph m_graph;
    vertices_draw_cont_t m_vertices;
    edges_draw_cont_t m_edges;
    ATLADD com_ptr<ID2D1StrokeStyle1> m_areaStrokeStyle;
    UINT m_dpiChangedMessage;

#if defined(_DEBUG) || defined(SHOW_FPS)
    static constexpr size_t m_framesMaxNumber = 100;
    // `m_frameIt` must be declared after `m_frameTimes`! See member initialization list in the ctor.
    typedef std::vector<
        std::chrono::high_resolution_clock::duration::rep,
        STLADD default_allocator<std::chrono::high_resolution_clock::duration::rep>> frames_cont_t;
    frames_cont_t m_frameTimes;
    frames_cont_t::iterator m_frameIt;
    std::chrono::high_resolution_clock::duration::rep m_frameTotal;
    ATLADD com_ptr<IDWriteTextFormat> m_framesPerSecondTextFormat;
    ATLADD com_ptr<ID2D1SolidColorBrush> m_framesPerSecondBrush;
#endif
};

ATLADD_END
