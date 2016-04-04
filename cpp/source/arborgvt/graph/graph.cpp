#include "barnhut\barnhut.h"
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
        STLADD lock_guard_shared<WAPI srw_lock> edgesLock {m_edgesLock};
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
        STLADD lock_guard_exclusive<WAPI srw_lock> edgesLock {m_edgesLock};
        m_edges.emplace_back(new edge {tailVertex, headVertex, length, m_stiffness, true});
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
    m_meanOfEnergy = 0.0f;
    sse_t value = {m_distribution.a(), m_distribution.a(), m_distribution.b(), m_distribution.b()};
    m_graphBound = _mm_load_ps(value.data);
    m_viewBound = getZeroVector();
    STLADD lock_guard_exclusive<WAPI srw_lock> verticesLock {m_verticesLock};
    STLADD lock_guard_exclusive<WAPI srw_lock> edgesLock {m_edgesLock};
    m_edges.clear();
    m_vertices.clear();
}


/**
 * Updates physical and geometric parameters of this graph (moves to the next animation step).
 *
 * Parameters:
 * >renderSurfaceSize
 * Size (in DIPs, logical size) of a surface where this graph is rendered.
 *
 * Returns:
 * N/A.
 *
 * Remarks:
 * This is analogue of `ArborGVT::ArborSystem::tickTimer` method in the original C# code.
 */
void graph::update(_In_ const __m128 renderSurfaceSize)
{
    STLADD lock_guard_exclusive<WAPI srw_lock> verticesLock {m_verticesLock, std::try_to_lock};
    if (verticesLock)
    {
        STLADD lock_guard_exclusive<WAPI srw_lock> edgesLock {m_edgesLock, std::try_to_lock};
        if (edgesLock)
        {
            updatePhysics();
            updateViewBound(renderSurfaceSize);
        }
    }
}


/**
 * Adds a new vertex to the graph if the latter doesn't have a vertex with the same name.
 *
 * Parameters:
 * >name
 * New vertex name.
 * >bkgndColor
 * Vertex background color.
 * >textColor
 * Vertex text color.
 * >mass
 * Vertex mass.
 * >fixed
 * Vertex movement ability.
 *
 * Returns:
 * Pointer to the new vertex instance.
 *
 * Remarks:
 * This method obtains exclusive lock on the `m_verticesLock` mutex only.
 *
 * To exploit `graph::addVertex(_In_ STLADD string_type&&)` method, this one calls that method. And that's why (because
 * a vertex requires random coordinates) I didn't create new vertex class ctor.
 */
vertex* graph::addVertex(
    _In_ STLADD string_type&& name,
    _In_ const D2D1_COLOR_F& bkgndColor,
    _In_ const D2D1_COLOR_F& textColor,
    _In_ const float mass,
    _In_ const bool fixed)
{
    STLADD lock_guard_exclusive<WAPI srw_lock> verticesLock {m_verticesLock};
    vertex* v = addVertex(std::move(name));
    sse_t value = {bkgndColor.r, bkgndColor.g, bkgndColor.b, bkgndColor.a};
    v->setColor(_mm_load_ps(value.data));
    value = {textColor.r, textColor.g, textColor.b, textColor.a};
    v->setTextColor(_mm_load_ps(value.data));
    value.data[0] = mass;
    __m128 temp = _mm_load_ps(value.data);
    v->setMass(_mm_shuffle_ps(temp, temp, 0));
    v->setFixed(fixed);
    return v;
}


/**
 * Adds a new edge to the graph. The edge connects two specified vertices.
 *
 * Parameters:
 * >tail
 * Tail vertex, where the new edge begins.
 * >head
 * Head vertex, where the new edge ends.
 * >length
 * Size of the new edge.
 * >stiffness
 * New edge stiffness.
 * >directed
 * Determines edge style: is it directed or not.
 * >color
 * Edge drawing color.
 *
 * Returns:
 * Pointer to the new edge instance.
 *
 * Remarks:
 * This method obtains exclusive lock on the `m_edgesLock` mutex only.
 */
edge* graph::addEdge(
    _In_ vertex* tail,
    _In_ vertex* head,
    _In_ const float length,
    _In_ const float stiffness,
    _In_ const bool directed,
    _In_ const D2D1_COLOR_F& color)
{
    STLADD lock_guard_exclusive<WAPI srw_lock> edgesLock {m_edgesLock};
    m_edges.emplace_back(new edge {tail, head, length, stiffness, directed, color});
    return m_edges.back().get();
}


/**
 * Adds a new edge to the graph. The edge connects two specified vertices.
 *
 * Parameters:
 * >tail
 * Tail vertex, where the new edge begins.
 * >head
 * Head vertex, where the new edge ends.
 * >length
 * Size of the new edge.
 * >directed
 * Determines edge style: is it directed or not.
 * >color
 * Edge drawing color.
 *
 * Returns:
 * Pointer to the new edge instance.
 *
 * Stiffness of the new edge is assigned by this graph's 'stifness' setting.
 *
 * Remarks:
 * This method obtains exclusive lock on the `m_edgesLock` mutex only.
 */
edge* graph::addEdge(
    _In_ vertex* tail,
    _In_ vertex* head,
    _In_ const float length,
    _In_ const bool directed,
    _In_ const D2D1_COLOR_F& color)
{
    return addEdge(tail, head, length, m_stiffness, directed, color);
}


/**
 * Adds a new vertex to the graph if the latter doesn't have a vertex with the same name.
 *
 * Parameters:
 * >name
 * New vertex name.
 *
 * Returns:
 * Pointer to the new vertex instance.
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
    std::random_device rd {};
    std::mt19937 engine {rd()};
    sse_t value = {m_distribution(engine), m_distribution(engine), 0.0f, 0.0f};
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
 * Pointer to the new vertex instance.
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
     *     m_vertices.insert({STLADD string_type {name}, vertex {std::move(name), coordinates}});
     *
     * Thus, the above code for 'VC 2015 Update 2' calls this template ctor of `std::pair`:
     * std::pair(std::basic_string<...>& _Val1, arbor::vertex&& _Val2 = {...});
     * with arguments deduced as:
     *     - for template ctor itself: <std::basic_string<...>& __ptr64, arbor::vertex, void>,
     *     - for `pair` template class: <std::basic_string<...> const, arbor::vertex>.
     * Following [8.5.4.4] the code, at first, makes a copy of `name` argument (passing it as `_Val1` parameter),
     * and after that the code moves `name` into `vertex` ctor.
     */
    std::pair<vertices_cont_t::iterator, bool> result =
        m_vertices.insert({STLADD string_type {name}, vertex {std::move(name), coordinates}});
    return &(result.first->second);
}


/**
 * Recalculates bounds of the rectangle containing all vertices. New rectangle is stored as updated `m_graphBound`.
 *
 * Parameters:
 * None.
 *
 * Returns:
 * N/A.
 *
 * Remarks:
 * This method DOES NOT obtain any lock while it accesses the graph's vertices. Caller of this method must guarantee
 * that other threads can't access graph's data while this method works. This method doesn't lock the vertices because
 * edges and vertices locks must be obtained in the specific order only (see remarks section for the `graph::addEdge`
 * method), but noone knows when this method will be called (after of before edges lock was/will be obtained).
 *
 * This is `ArborGVT::ArborSystem::updateGraphBounds` method in the original C# code.
 */
void graph::updateGraphBound()
{
    sse_t value;
    value.data[0] = m_distribution.a();
    value.data[1] = m_distribution.b();
    __m128 temp = _mm_load_ps(value.data);
    temp  = _mm_shuffle_ps(temp, temp, 0b01010000);
    for (auto it = m_vertices.cbegin(); m_vertices.cend() != it; ++it)
    {
        __m128 coordinate = it->second.getCoordinates();
        if (0b0011 == (0b0011 & _mm_movemask_ps(_mm_cmpeq_ps(coordinate, coordinate))))
        {
            int compare = _mm_movemask_ps(_mm_cmplt_ps(coordinate, temp));
            if (0b0001 & compare)
            {
                __m128 temp2 = _mm_shuffle_ps(coordinate, temp, 0b01100100);
                temp = _mm_shuffle_ps(temp2, temp, 0b11101100);
            }
            if (0b0010 & compare)
            {
                __m128 temp2 = _mm_shuffle_ps(coordinate, temp, 0b00100100);
                temp = _mm_shuffle_ps(temp2, temp, 0b11100111);
            }
            temp = _mm_shuffle_ps(temp, temp, 0b01001110);
            compare = _mm_movemask_ps(_mm_cmpgt_ps(coordinate, temp));
            if (0b0001 & compare)
            {
                __m128 temp2 = _mm_shuffle_ps(coordinate, temp, 0b01100100);
                temp = _mm_shuffle_ps(temp2, temp, 0b11101100);
            }
            if (0b0010 & compare)
            {
                __m128 temp2 = _mm_shuffle_ps(coordinate, temp, 0b00100100);
                temp = _mm_shuffle_ps(temp2, temp, 0b11100111);
            }
            temp = _mm_shuffle_ps(temp, temp, 0b01001110);
        }
    }
    m_graphBound = temp;
}


/**
 * Makes animation step. This method increments the "view area" by one step toward the "graph area".
 *
 * Parameters:
 * >renderSurfaceSize
 * Size (in DIPs, logical size) of a surface where this graph is rendered.
 *
 * Returns:
 * N/A.
 *
 * Remarks:
 * This method calls `updateGraphBound` and also doesn't obtain any lock.
 *
 * This is `ArborGVT::ArborSystem::updateViewBounds` method in the original C# code.
 */
void graph::updateViewBound(_In_ const __m128 renderSurfaceSize)
{
    updateGraphBound();
    if (0b1111 != (0b1111 & _mm_movemask_ps(_mm_cmpeq_ps(m_viewBound, getZeroVector()))))
    {
        __m128 temp = _mm_sub_ps(m_graphBound, m_viewBound);
        sse_t value;
        value.data[0] = m_animationStep;
        __m128 temp2 = _mm_load_ps(value.data);
        temp2 = _mm_shuffle_ps(temp2, temp2, 0);
        __m128 delta = _mm_mul_ps(temp, temp2);
        __m128 leftTop;
        __m128 rightBottom;
        if (simd_cpu_capabilities::sse41())
        {
            leftTop = _mm_dp_ps(delta, delta, 0b00111111);
            rightBottom = _mm_dp_ps(delta, delta, 0b11001111);
        }
        else
        {
            leftTop = _mm_shuffle_ps(delta, delta, 0b01000100);
            leftTop = _mm_mul_ps(leftTop, leftTop);
            leftTop = _mm_add_ps(leftTop, _mm_shuffle_ps(leftTop, leftTop, 0b10110001));
            rightBottom = _mm_shuffle_ps(delta, delta, 0b11101110);
            rightBottom = _mm_mul_ps(rightBottom, rightBottom);
            rightBottom = _mm_add_ps(rightBottom, _mm_shuffle_ps(rightBottom, rightBottom, 0b10110001));
        }
        temp = _mm_shuffle_ps(leftTop, rightBottom, 0b01000100);
        temp = _mm_shuffle_ps(temp, temp, 0b11011000);
        temp = _mm_sqrt_ps(temp);
        temp = _mm_mul_ps(temp, renderSurfaceSize);
        value.data[0] = 1.0f;
        temp2 = _mm_load_ps(value.data);
        temp2 = _mm_shuffle_ps(temp2, temp2, 0);
        if (0b0011 & _mm_movemask_ps(_mm_cmpgt_ps(temp, temp2)))
        {
            m_viewBound = _mm_add_ps(m_viewBound, delta);
        }
    }
    else
    {
        m_viewBound = m_graphBound;
    }
}


/**
 * Updates physical parameters of this graph (moves to the next animation step).
 *
 * Parameters:
 * None.
 *
 * Returns:
 * N/A.
 *
 * Remarks:
 * This method obtains no lock.
 *
 * This is `ArborGVT::ArborSystem::updatePhysics` method in the original C# code.
 */
void graph::updatePhysics()
{
    // > Tend particles.
    __m128 zero = getZeroVector();
    for (auto it = m_vertices.begin(); m_vertices.end() != it; ++it)
    {
        it->second.setVelocity(zero);
    }
//    if (0 < m_stiffness) -- these are "warning C4127: conditional expression is constant".
    {
        applySprings();
    }
    // > Euler integrator.
//    if (0 < m_repulsion)
    {
        applyBarnesHutRepulsion();
    }
    updateVelocityAndPosition(m_timeSlice);
}


/**
 * Creates Barnes Hut simulation over this graph's vertices.
 *
 * Parameters:
 * None.
 *
 * Returns:
 * N/A.
 *
 * Remarks:
 * This method obtains no lock.
 *
 * This is `ArborGVT::ArborSystem::applyBarnesHutRepulsion` method in the original C# code.
 */
void graph::applyBarnesHutRepulsion()
{
    BHUT barnes_hut_tree simulation {m_graphBound, m_theta};
    for (auto it = m_vertices.begin(); m_vertices.end() != it; ++it)
    {
        simulation.insert(&(it->second));
    }
    for (auto it = m_vertices.begin(); m_vertices.end() != it; ++it)
    {
        simulation.applyForce(&(it->second), m_repulsion);
    }
}


/**
 * Changes forces, applied to both vertices of each edge.
 *
 * Parameters:
 * None.
 *
 * Returns:
 * N/A.
 *
 * Remarks:
 * This method DOES NOT obtain any lock while it accesses the graph's edges. Caller of this method must guarantee that
 * other threads can't access graph's data while this method works. This method doesn't lock the edges because edges
 * and vertices locks must be obtained in the specific order only (see remarks section for the `graph::addEdge` method),
 * but noone knows when this method will be called (after of before vertices lock was/will be obtained).
 *
 * This is `ArborGVT::ArborSystem::applySprings` method in the original C# code.
 */
void graph::applySprings()
{
    __m128 zero = getZeroVector();
    for (auto it = m_edges.begin(); m_edges.end() != it; ++it)
    {
        vertex* tail = (*it)->getTail();
        vertex* head = (*it)->getHead();
        __m128 temp = _mm_sub_ps(head->getCoordinates(), tail->getCoordinates());
        __m128 temp2;
        if (simd_cpu_capabilities::sse41())
        {
            temp2 = _mm_dp_ps(temp, temp, 0b00111111);
        }
        else
        {
            temp2 = _mm_shuffle_ps(temp, temp, 0b01000100);
            temp2 = _mm_mul_ps(temp2, temp2);
            temp2 = _mm_add_ps(temp2, _mm_shuffle_ps(temp2, temp2, 0b10110001));
        }
        temp2 = _mm_sqrt_ps(temp2);
        __m128 oldSize = temp2;
        if (0b1111 & _mm_movemask_ps(_mm_cmpeq_ps(temp2, zero)))
        {
            temp = randomVector(1.0f);
            if (simd_cpu_capabilities::sse41())
            {
                temp2 = _mm_dp_ps(temp, temp, 0b00111111);
            }
            else
            {
                temp2 = _mm_mul_ps(temp, temp);
                temp2 = _mm_add_ps(temp2, _mm_shuffle_ps(temp2, temp2, 0b10110001));
            }
            temp2 = _mm_rsqrt_ps(temp2);
        }
        else
        {
            temp2 = _mm_rcp_ps(temp2);
        }
        temp = _mm_mul_ps(temp, temp2);
        sse_t value;
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


/**
 * Updates velocity and position of each vertex in this graph. A force applied to each vertex is zeroed.
 *
 * Parameters:
 * >time
 * Constant time slice between two consequent updates?
 *
 * Returns:
 * N/A.
 *
 * Remarks:
 * This method DOES NOT obtain any lock while it accesses the graph's vertices. Caller of this method must guarantee
 * that other threads can't access graph's data while this method works. This method doesn't lock the vertices because
 * edges and vertices locks must be obtained in the specific order only (see remarks section for the `graph::addEdge`
 * method), but noone knows when this method will be called (after of before edges lock was/will be obtained).
 *
 * This is `ArborGVT::ArborSystem::updateVelocityAndPosition` method in the original C# code.
 */
void graph::updateVelocityAndPosition(_In_ const float time)
{
    if (!m_vertices.size())
    {
        m_meanOfEnergy = 0.0f;
        return;
    }

    /*
     * > Calculate center drift.
     */
    __m128 zero = getZeroVector();
    __m128 energyTotal = zero;
    __m128 drift = zero;
    for (auto it = m_vertices.cbegin(); m_vertices.cend() != it; ++it)
    {
        drift = _mm_sub_ps(drift, it->second.getCoordinates());
    }
    sse_t value;
    value.data[0] = static_cast<float> (m_vertices.size());
    __m128 size = _mm_load_ps(value.data);
    size = _mm_shuffle_ps(size, size, 0);
    drift = _mm_mul_ps(drift, _mm_rcp_ps(size));
    /*
     * > Main updates loop.
     *
     * Initialize loop invariants.
     */
    __m128 repulsion;
    if (m_gravity)
    {
        value.data[0] = m_repulsion * (-0.01f);
        repulsion = _mm_load_ps(value.data);
        repulsion = _mm_shuffle_ps(repulsion, repulsion, 0);
    }
    /*
     * Original C# code compares `velocity` vector length against the predefined constant (1'000). To avoid
     * redundant square root op (SQRTPS) I use `velocity * velocity` dot product as is and compare it against
     * 1'000'000 value.
     *
     * lim (x * x) = 1'000'000
     * x -> 1'000
     */
#if defined(__ICL)
    value.data[0] = 1000000.0f;
#else
    value.data[0] = 1'000'000.0f;
#endif
    __m128 velocityVectorLength = _mm_load_ps(value.data);
    velocityVectorLength = _mm_shuffle_ps(velocityVectorLength, velocityVectorLength, 0);
    value.data[0] = 1.0f - m_friction;
    __m128 frictionCompVector = _mm_load_ps(value.data);
    frictionCompVector = _mm_shuffle_ps(frictionCompVector, frictionCompVector, 0);
    value.data[0] = time;
    __m128 timeVector = _mm_load_ps(value.data);
    timeVector = _mm_shuffle_ps(timeVector, timeVector, 0);
    __m128 temp;
    for (auto it = m_vertices.begin(); m_vertices.end() != it; ++it)
    {
        // > Apply center drift.
        it->second.applyForce(drift);
        // > Apply center gravity.
        if (m_gravity)
        {
            temp = _mm_mul_ps(it->second.getCoordinates(), repulsion);
            it->second.applyForce(temp);
        }
        // > Update velocity.
        if (!it->second.getFixed())
        {
            temp = _mm_mul_ps(it->second.getForce(), timeVector);
            __m128 velocity = _mm_add_ps(it->second.getVelocity(), temp);
            velocity = _mm_mul_ps(velocity, frictionCompVector);
            if (simd_cpu_capabilities::sse41())
            {
                temp = _mm_dp_ps(velocity, velocity, 0b00111111);
            }
            else
            {
                temp = _mm_shuffle_ps(velocity, velocity, 0b01000100);
                temp = _mm_mul_ps(temp, temp);
                temp = _mm_add_ps(temp, _mm_shuffle_ps(temp, temp, 0b10110001));
            }
            if (0b1111 & _mm_movemask_ps(_mm_cmpgt_ps(temp, velocityVectorLength)))
            {
                velocity = _mm_mul_ps(velocity, _mm_rcp_ps(temp));
            }
            it->second.setVelocity(velocity);
        }
        else
        {
            it->second.setVelocity(zero);
        }
        it->second.setForce(zero);
        // > Update positions.
        __m128 velocity = it->second.getVelocity();
        temp = _mm_mul_ps(velocity, timeVector);
        temp = _mm_add_ps(it->second.getCoordinates(), temp);
        it->second.setCoordinates(temp);
        // > Update energy.
        if (simd_cpu_capabilities::sse41())
        {
            temp = _mm_dp_ps(velocity, velocity, 0b00111111);
        }
        else
        {
            temp = _mm_shuffle_ps(velocity, velocity, 0b01000100);
            temp = _mm_mul_ps(temp, temp);
            temp = _mm_add_ps(temp, _mm_shuffle_ps(temp, temp, 0b10110001));
        }
        // The following dot product: `velocity` * `velocity` gives energy? Never knew.
        energyTotal = _mm_add_ps(energyTotal, temp);
    }
    temp = _mm_mul_ps(energyTotal, _mm_rcp_ps(size));
    _mm_store_ps(value.data, temp);
    m_meanOfEnergy = value.data[0];
}

ARBOR_END
