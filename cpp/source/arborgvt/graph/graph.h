#pragma once
#include "ns\arbor.h"
#include "graph\edge.h"
#include "graph\vector.h"
#include "graph\vertex.h"
#include "service\sse.h"
#include "service\stladdon.h"
#include "service\winapi\srwlock.h"
#include <memory>
#include <random>
#include <unordered_map>
#include <vector>

ARBOR_BEGIN

/**
 * `graph` implements a graph.
 *
 * Remember that in the current state this implementation mostly reproduces a model taken from the C# code base.
 *
 * Remarks:
 * Vertices of the graph are stored inside `std::unordered_map` that uses `std::pair` to store key/value pair. 'Value'
 * is an instance of `ARBOR vertex` class that has a 16-byte alignment. Therefore `std::pair<T, ARBOR vertex>` also will
 * have a properly alignment (16 or multiple). But I still have to use `STLADD aligned_allocator` to store pairs in the
 * map. Because default STL allocator's `allocate` member function allocates a memory without knowing what C++ type
 * will be construct in that memory.
 *
 * Edges are stored inside `std::vector` container as objects wrapped by `std::unique_ptr`s. Each edge object must also
 * be aligned on a 16-byte boundary. This is guaranteed by `edge` instance itself (with overloaded `new` and `delete`
 * operators).
 *
 * Just because I'm "copying" from the C# source code base I'm adding to the `graph` class methods that do some physical
 * calculations. Logically it's a part of another class, but I'm making `graph` class just like Csharp's `ArborSystem`.
 */
class graph_data_type
{
protected:
    typedef std::unordered_map<
        STLADD string_type,
        vertex,
        std::hash<STLADD string_type>,
        std::equal_to<STLADD string_type>,
        STLADD aligned_sse_allocator<std::pair<const STLADD string_type, vertex>>> vertices_cont_t;
    typedef std::vector<std::unique_ptr<edge>, STLADD default_allocator<std::unique_ptr<edge>>> edges_cont_t;

    // Prepare tag dispatch pattern.
    template <typename T>
    struct is_pair_iterator_type: public std::false_type
    {
    };

    template<>
    struct is_pair_iterator_type<vertices_cont_t::iterator>: public std::true_type
    {
    };

    template<>
    struct is_pair_iterator_type<vertices_cont_t::const_iterator>: public std::true_type
    {
    };

    template <bool>
    struct is_pair_type
    {
    };
    typedef is_pair_type<true> is_pair_t;
    typedef is_pair_type<false> is_not_pair_t;

    template <typename T, typename U>
    static T getReference(_In_ U value, is_pair_t) noexcept
    {
        return value->second;
    }

    template <typename T, typename U>
    static T getReference(_In_ U value, is_not_pair_t) noexcept
    {
        return *value;
    }

    template <typename T, typename U>
    static T getPointer(_In_ U value, is_pair_t) noexcept
    {
        return &(value->second);
    }

    template <typename T, typename U>
    static T getPointer(_In_ U value, is_not_pair_t) noexcept
    {
        return value->get();
    }

    // Iterator adaptor, used to hide real types of containers.
    template <typename T, typename U>
    class data_iterator: public std::iterator<typename U::iterator_category, T>
    {
    public:
        explicit data_iterator(_In_ U value) noexcept
            :
            m_value {value}
        {
        }

        bool operator !=(_In_ const data_iterator& right) const noexcept
        {
            return  m_value != right.m_value;
        }

        data_iterator& operator ++()
        {
            ++m_value;
            return *this;
        }

        reference operator *() const noexcept
        {
            return getReference<reference>(m_value, is_pair_type<is_pair_iterator_type<U>::value> {});
        }

        pointer operator ->() const noexcept
        {
            return getPointer<pointer>(m_value, is_pair_type<is_pair_iterator_type<U>::value> {});
        }


    private:
        U m_value;
    };
};

class graph: private graph_data_type
{
public:
    typedef data_iterator<vertices_cont_t::mapped_type, vertices_cont_t::iterator> vertices_iterator;
    typedef data_iterator<const vertices_cont_t::mapped_type, vertices_cont_t::const_iterator> const_vertices_iterator;
    typedef data_iterator<edges_cont_t::value_type, edges_cont_t::iterator> edges_iterator;
    typedef data_iterator<const edges_cont_t::value_type, edges_cont_t::const_iterator> const_edges_iterator;

    graph()
        :
        m_vertices {},
        m_edges {},
        m_distribution {-2.0f, 2.0f},
        m_verticesLock {},
        m_edgesLock {},
        m_meanOfEnergy {0.0f}
    {
        sse_t value = {m_distribution.a(), m_distribution.a(), m_distribution.b(), m_distribution.b()};
        m_graphBound = _mm_load_ps(value.data);
        m_viewBound = getZeroVector();
    }

    static void* operator new(_In_ const size_t size)
    {
        STLADD aligned_sse_allocator<graph> allocator {};
        return allocator.allocate(size, 0);
    }

    static void operator delete(_In_ void* p)
    {
        STLADD aligned_sse_allocator<graph> allocator {};
        allocator.deallocate(p);
    }

    WAPI srw_lock& getVerticesLock() noexcept
    {
        return m_verticesLock;
    }

    WAPI srw_lock& getEdgesLock() noexcept
    {
        return m_edgesLock;
    }

    void addEdge(_In_ STLADD string_type&& tail, _In_ STLADD string_type&& head, _In_ float length);
    void clear() noexcept;

    auto verticesBegin() noexcept
    {
        return vertices_iterator {m_vertices.begin()};
    }

    auto verticesBegin() const noexcept
    {
        return const_vertices_iterator {m_vertices.begin()};
    }

    auto verticesEnd() noexcept
    {
        return vertices_iterator {m_vertices.end()};
    }

    auto verticesEnd() const noexcept
    {
        return const_vertices_iterator {m_vertices.end()};
    }

    auto edgesBegin() noexcept
    {
        return edges_iterator {m_edges.begin()};
    }

    auto edgesBegin() const noexcept
    {
        return const_edges_iterator {m_edges.begin()};
    }

    auto edgesEnd() noexcept
    {
        return edges_iterator {m_edges.end()};
    }

    auto edgesEnd() const noexcept
    {
        return const_edges_iterator {m_edges.end()};
    }

    bool active() const noexcept
    {
        if (m_autoStop)
        {
            return (m_energyThreshold < m_meanOfEnergy) || (0.0f == m_meanOfEnergy);
        }
        else
        {
            return true;
        }
    }

    __m128 __vectorcall getViewBound() const noexcept
    {
        return m_viewBound;
    }

    void __vectorcall update(_In_ const __m128 renderSurfaceSize);


private:
    vertex* addVertex(_In_ STLADD string_type&& name);
    vertex* __vectorcall addVertex(_In_ STLADD string_type&& name, _In_ const __m128 coordinates);
    void updateGraphBound();
    void __vectorcall updateViewBound(_In_ const __m128 renderSurfaceSize);
    void updatePhysics();
    void applyBarnesHutRepulsion();
    void applySprings();
    void __fastcall updateVelocityAndPosition(_In_ const float time);

    static constexpr float m_stiffness = 250.0f;
#if defined(__ICL)
    // Intel C++ 16.0 doesn't support single-quotation mark as a digit separator for floating point types,
    // it's a known bug with internal tracker DPD200379927.
    static constexpr float m_repulsion = 10000.0f;
#else
    static constexpr float m_repulsion = 10'000.0f;
#endif
    static constexpr float m_friction = 0.1f;
    static constexpr float m_animationStep = 0.04f;
    static constexpr float m_timeSlice = 0.01f;
    static constexpr float m_energyThreshold = 0.7f;
    static constexpr float m_theta = 0.4f;
    static constexpr bool m_gravity = false;
    static constexpr bool m_autoStop = false;
    /*
     * `m_graphBound` is the area used by Barnes Hut algorithm. This is a coordinate space where all graph vertices
     * exist (coordinates of any vertex count in this space).
     * `m_viewBound` is an area inside `m_graphBound`. This area is mapped onto HWND, where this graph is rendered.
     *
     * While physics calculation is active, the `m_viewBound` is always seeking to become as large as the
     * `m_graphBound` is. Size of the `m_viewBound` is inversely proportional to size of target HWND (where
     * `m_viewBound` is mapped). Therefore while the `m_viewBound` is growing, vertices are moving from outside of the
     * HWND toward the center of its client area.
     *
     * This class exposes the `m_viewBound` area to caller, which must transform a vertex coordinate from `m_graphBound`
     * coordinate space to its own one (HWND client area, for example). Remember: the `m_viewBound` is just a way to
     * animate initial movement of graph vertices from outside of the HWND inward the HWND client area. That's why the
     * `m_viewBound` grows, that's why the `m_viewBound` and the target render area are inversely proportional.
     *
     * You can avoid using it and implement that movement by itself (with WAM, for example). But this class will do
     * another animation anyway (a one for Barnes Hut algorithm). Therefore you will end up with two or even more
     * animation sequences.
     *
     * Names suffix `...Bound` came from the C# source code.
     *
     * Both the vectors formatted as [bottom-y, right-x, top-y, left-x].
     */
    __m128 m_graphBound;
    __m128 m_viewBound;
    vertices_cont_t m_vertices;
    edges_cont_t m_edges;
    std::uniform_real_distribution<float> m_distribution;
    WAPI srw_lock m_verticesLock;
    WAPI srw_lock m_edgesLock;
    float m_meanOfEnergy;
};

ARBOR_END
