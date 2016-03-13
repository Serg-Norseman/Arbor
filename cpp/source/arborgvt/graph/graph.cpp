#include "graph\graph.h"
#include "graph\vector.h"

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
 *
 * Remarks:
 * The method exclusively locks the `m_verticesLock` mutex first and then acquires the `m_edgesLock` mutex. If another
 * thread, using another method of this class, will obtain the locks in a different order, a deadlock may occur. To
 * avoid it all threads must use the same order when they acquire the locks or try-to-acquire forms of lock must be
 * used (with `std::try_to_lock` argument).
 */
void graph::addEdge(_In_ STLADD string_type&& tail, _In_ STLADD string_type&& head, _In_ float length)
{
    STLADD lock_guard_exclusive<WAPI srw_lock> verticesLock {m_verticesLock};
    vertex* tailVertex = addVertex(std::move(tail));
    vertex* headVertex = addVertex(std::move(head));
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
        edge* p = new edge {tailVertex, headVertex, true, length, m_stiffness};
        STLADD lock_guard_exclusive<WAPI srw_lock> lock {m_edgesLock};
        m_edges.emplace_back(p);
    }
}


/**
 * Removes all edges and vertices from this graph.
 *
 * Parameters:
 * None.
 *
 * Returns:
 * N/A.
 */
void graph::clear() noexcept
{
    STLADD lock_guard_exclusive<WAPI srw_lock> verticesLock {m_verticesLock};
    STLADD lock_guard_exclusive<WAPI srw_lock> lock {m_edgesLock};
    m_edges.clear();
    m_vertices.clear();
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
vertex* graph::addVertex(_In_ STLADD string_type&& name)
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
 * Therefore, BE CAREFUL: this method doesn't obtain any locks! It totally relies on caller.
 */
vertex* __vectorcall graph::addVertex(_In_ STLADD string_type&& name, _In_ const __m128 coordinates)
{
    /*
     * About order of evaluation of inside initializer-list.
     * >8.5.4 List-initialization [dcl.init.list]
     * >4 Within the initializer-list of a braced-init-list, the initializer-clauses, including any that result from
     * >pack expansions (14.5.3), are evaluated in the order in which they appear. That is, every value computation and
     * >side effect associated with a given initializer-clause is sequenced before every value computation and
     * >side effect associated with any initializer-clause that follows it in the comma-separated list of the
     * >initializer-list. [Note: This evaluation ordering holds regardless of the semantics of the initialization;
     * >for example, it applies when the elements of the initializer-list are interpreted as arguments of a constructor
     * >call, even though ordinarily there are no sequencing constraints on the arguments of a call. -end note].
     *
     * If MSVC 2015 Update 2 will really consider this as it's expected (MSVC before 2015 Update 2 release doesn't),
     * the following code can be changed like this:
     * // For MSVC 2015 Update 1 and below:
     * STLADD string_type key {name};
     * std::pair<vertices_cont_t::iterator, bool> result =
     *     m_vertices.emplace(key, vertex {std::move(name), coordinates});
     *
     * // For MSVC 2015 Update 2 and above:
     * std::pair<vertices_cont_t::iterator, bool> result =
     *     m_vertices.insert({name, vertex {std::move(name), coordinates}});
     *
     * Thus, the above code for 'VC 2015 Update 2' calls this template ctor of `std::pair`:
     * std::pair(std::basic_string<...>& _Val1, arbor::vertex&& _Val2 = {...});
     * with arguments deduced as:
     *     - for template ctor itself: <std::basic_string<...>& __ptr64, arbor::vertex, void>,
     *     - for `pair` template class: <std::basic_string<...> const, arbor::vertex>.
     * Following [8.5.4.4] the code, at first, makes a copy of `name` argument (passing it as `_Val1` parameter),
     * and after that the code moves `name` into `vertex` ctor.
     */
    STLADD string_type key {name};
    std::pair<vertices_cont_t::iterator, bool> result =
        m_vertices.emplace(key, vertex {std::move(name), coordinates});
    return &(result.first->second);
}


/**
 * Change forces, applied to both vertices of each edge.
 *
 * Parameters:
 * None.
 *
 * Returns:
 * N/A.
 *
 * Remarks:
 * This is `ArborGVT::ArborSystem::applySprings` method in the original C# code.
 */
void graph::applySprings()
{
    sse_t value;
    value.data[0] = 0;
    __m128 zero = _mm_load_ps(value.data);
    zero = _mm_shuffle_ps(zero, zero, 0);
    for (auto it = m_edges.begin(); m_edges.end() != it; ++it)
    {
        vertex* tail = (*it)->getTail();
        vertex* head = (*it)->getHead();
        __m128 temp = _mm_sub_ps(head->getCoordinates(), tail->getCoordinates());
        __m128 temp2;
        if (simd_cpu_capabilities::sse41())
        {
            temp2 = _mm_dp_ps(temp, temp, 0b00110001);
        }
        else
        {
            temp2 = _mm_mul_ps(temp, temp);
            temp2 = _mm_hadd_ps(temp2, temp2);
        }
        temp2 = _mm_sqrt_ss(temp2);
        temp2 = _mm_shuffle_ps(temp2, temp2, 0);
        __m128 oldSize = temp2;
        if (0b0001 & _mm_movemask_ps(_mm_cmpeq_ps(temp2, zero)))
        {
            temp = randomVector(1.0f);
            if (simd_cpu_capabilities::sse41())
            {
                temp2 = _mm_dp_ps(temp, temp, 0b00110001);
            }
            else
            {
                temp2 = _mm_mul_ps(temp, temp);
                temp2 = _mm_hadd_ps(temp2, temp2);
            }
            temp2 = _mm_sqrt_ss(temp2);
            temp2 = _mm_shuffle_ps(temp2, temp2, 0);
        }
        temp = _mm_mul_ps(temp, _mm_rcp_ps(temp2));
        value.data[0] = (*it)->getLength();
        temp2 = _mm_load_ps(value.data);
        temp2 = _mm_shuffle_ps(temp2, temp2, 0);
        oldSize = _mm_sub_ps(temp2, oldSize);
        value.data[0] = (*it)->getStiffness() * 0.5f;
        temp2 = _mm_load_ps(value.data);
        temp2 = _mm_shuffle_ps(temp2, temp2, 0);
        temp2 = _mm_mul_ps(temp2, oldSize);
        temp = _mm_mul_ps(temp, temp2);
        head->applyForce(temp);
        temp = _mm_sub_ps(zero, temp);
        tail->applyForce(temp);
    }
}

ARBOR_END
