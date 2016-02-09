#include "graph\graph.h"

ARBOR_BEGIN
/**
 * Adds a new edge to the graph. The edge connects two specified vertices.
 *
 * Parameters:
 * >tail
 * Name of the tail vertex, where the new edge begins.
 * >head
 * Name of the head vertex, where the new edge ends.
 * >length
 * Size of the new edge.
 *
 * Returns:
 * N/A.
 */
void graph::addEdge(_In_ STLADD string_type&& tail, _In_ STLADD string_type&& head, _In_ float length)
{
    STLADD lock_guard_exclusive<WAPI srw_lock> verticesLock {m_verticesLock};
    const vertex* tailVertex = addVertex(std::move(tail));
    const vertex* headVertex = addVertex(std::move(head));
    bool noEdge = false;
    {
        STLADD lock_guard_shared<WAPI srw_lock> lock {m_edgesLock};
        noEdge = std::none_of(
            m_edges.cbegin(),
            m_edges.cend(),
            [tailVertex, headVertex] (_In_ const auto& value) -> bool
            {
                return (value->getTail() == tailVertex) && (value->getHead() == headVertex);
            });
    }
    if (noEdge)
    {
        STLADD default_allocator<edge> allocator {};
        edge* p = allocator.allocate(1);
        allocator.construct(p, tailVertex, headVertex, length, m_stiffness);
        STLADD lock_guard_exclusive<WAPI srw_lock> lock {m_edgesLock};
        m_edges.emplace_back(p);
    }
}


/**
 * Adds a new vertex to the graph if the latter doesn't have a vertex with the same name.
 *
 * Parameters:
 * >name
 * New vertex name.
 *
 * Returns:
 * Pointer to the vertex instance.
 *
 * Remarks:
 * The caller of this method MUST obtain exclusive lock on the `m_verticesLock` mutex. Because:
 * a) This method changes the internal vertices container,
 * b) This method returns pointer to a vertex, stored in the internal container. And at least shared lock must exist
 *    after this method has returned the pointer to the caller. But because I use SRW lock, that neither can be
 *    recursive nor can be upgraded from shared to exclusive mode, the caller has to obtain exclusive SRW lock from the
 *    beginning.
 *
 * Therefore, BE CAREFUL: this method doesn't obtain any locks! It totally relies on caller.
 */
const vertex* graph::addVertex(_In_ STLADD string_type&& name)
{
    // Here we can end up with issuing two unnecessary calls to "get a randomly distributed value".
    sse_t value = {m_distribution(m_engine), m_distribution(m_engine), 0.0f, 0.0f};
    return addVertex(std::move(name), _mm_load_ps(value.data));
}


/**
 * Adds a new vertex to the graph if the latter doesn't have a vertex with the same name.
 *
 * Parameters:
 * >name
 * New vertex name.
 * >coordinates
 * Coordinates of the new vertex.
 *
 * Returns:
 * Pointer to the vertex instance.
 *
 * Remarks:
 * The caller of this method MUST obtain exclusive lock on the `m_verticesLock` mutex. Because:
 * a) This method changes the internal vertices container,
 * b) This method returns pointer to a vertex, stored in the internal container. And at least shared lock must exist
 *    after this method has returned the pointer to the caller. But because I use SRW lock, that neither can be
 *    recursive nor can be upgraded from shared to exclusive mode, the caller has to obtain exclusive SRW lock from the
 *    beginning.
 *
 * Therefore, BE CAREFUL: this method doesn't obtain any locks! It totally relis on caller.
 */
const vertex* __vectorcall graph::addVertex(_In_ STLADD string_type&& name, _In_ const __m128 coordinates)
{
    // Don't obtain a shared lock here; caller has to do this.
    auto it = m_vertices.find(name);
    if (m_vertices.end() == it)
    {
        STLADD string_type key {name};
        // And no an exclusive lock here too.
        std::pair<vertices_cont_t::iterator, bool> result =
            m_vertices.emplace(key, vertex {std::move(name), coordinates});
        it = result.first;
    }
    return &(it->second);
}

ARBOR_END
