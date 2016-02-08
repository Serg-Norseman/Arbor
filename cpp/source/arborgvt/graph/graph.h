#pragma once
#include "ns\arbor.h"
#include "graph\edge.h"
#include "graph\vertex.h"
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
 * Vertices of the graph stored inside `std::unordered_map` that uses `std::pair` to store key/value pair. 'Value' is
 * an instance of `ARBOR vertex` class that has a 16-byte alignment. Therefore `std::pair<T, ARBOR vertex>` also will
 * have a properly alignment (16 or multiple). But I still have to use `ATLADD aligned_allocator` to store pairs in the
 * map. Because default STL allocator's `allocate` member function allocates a memory without knowing what C++ type
 * will be construct in that memory.
 */
class graph
{
public:
    graph()
        :
        m_vertices {},
        m_edges {},
        m_engine {},
        m_distribution {-1.0f, 1.0f},
        m_verticesLock {},
        m_edgesLock {}
    {
        std::random_device rd {};
        m_engine.seed(rd());
    }

    void addEdge(_In_ STLADD string_type&& tail, _In_ STLADD string_type&& head, _In_ float length);


private:
    typedef std::unordered_map <
        STLADD string_type,
        vertex,
        std::hash<STLADD string_type>,
        std::equal_to<STLADD string_type>,
        STLADD aligned_allocator<std::pair<const STLADD string_type, vertex>, alignof(__m128)>> vertices_cont_t;

    const vertex* addVertex(_In_ STLADD string_type&& name);
    const vertex* __vectorcall addVertex(_In_ STLADD string_type&& name, _In_ const __m128 coordinates);

    static constexpr float m_stiffness = 600.0f;

    vertices_cont_t m_vertices;
    std::vector<std::unique_ptr<edge, STLADD smart_ptr_deleter<edge>>> m_edges;
    std::mt19937 m_engine;
    std::uniform_real_distribution<float> m_distribution;
    WAPI srw_lock m_verticesLock;
    WAPI srw_lock m_edgesLock;
};

ARBOR_END
