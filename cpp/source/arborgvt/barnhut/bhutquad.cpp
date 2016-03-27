#include "barnhut\bhutquad.h"
#include "graph\vector.h"
#include "service\sse.h"

BHUT_BEGIN

/**
 * Increases parameters of the specified current branch using the specified source particle.
 *
 * Parameters:
 * >p
 * Particle owned by this branch. Parameters of this particle affects parameters of this branch.
 * >b
 * The current branch.
 * >quad
 * Quad of the `b` where `p` is located.
 * >particles
 * Container with particles to be handled.
 *
 * Returns:
 * Pointer to a branch to be set as the current one.
 *
 * Remarks:
 * This is `ArborGVT::BarnesHutTree::insert` method in the original C# code.
 */
branch* branch::handleParticle(_In_ const particle* p, _In_ branch* b, _In_ quad_index, _In_ particles_cont_t*)
{
    b->increaseParameters(p);
    return this;
}


/**
 * Applies forces to Barnes Hut tree's element.
 *
 * Parmeters:
 * >v
 * Graph vertex.
 * >repulsion
 * Repulsion setting.
 * >dist
 * Another one setting.
 * >elements
 * Tree's elements to iterate over.
 *
 * Returns:
 * N/A.
 *
 * Remarks:
 * This is `ArborGVT::BarnesHutTree::applyForces` method in the original C# code.
 */
void branch::applyForce(
    _In_ ARBOR vertex* v,
    _In_ const float repulsion,
    _In_ const float dist,
    _In_ quad_elements_cont_t* elements) const
{
    __m128 temp = _mm_rcp_ps(m_mass);
    temp = _mm_mul_ps(m_coordinates, temp);
    temp = _mm_sub_ps(v->getCoordinates(), temp);
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
    __m128 dotProduct = temp2;
    temp2 = _mm_sqrt_ps(temp2);
    __m128 temp3 = _mm_hsub_ps(m_area, m_area);
    temp3 = _mm_mul_ps(temp3, _mm_shuffle_ps(temp3, temp3, 0b10110001));
    temp3 = _mm_sqrt_ps(temp3);
    temp3 = _mm_mul_ps(temp3, _mm_rcp_ps(temp2));
    sse_t value;
    value.data[0] = dist;
    __m128 temp4 = _mm_load_ps(value.data);
    temp4 = _mm_shuffle_ps(temp4, temp4, 0);
    if (0b1111 == _mm_movemask_ps(_mm_cmpgt_ps(temp3, temp4)))
    {
        elements->push_back(m_quads.at(NorthEastQuad).get());
        elements->push_back(m_quads.at(NorthWestQuad).get());
        elements->push_back(m_quads.at(SouthEastQuad).get());
        elements->push_back(m_quads.at(SouthWestQuad).get());
    }
    else
    {
        value.data[0] = 1.0f;
        temp3 = _mm_load_ps(value.data);
        temp3 = _mm_shuffle_ps(temp3, temp3, 0);
        temp3 = _mm_max_ps(dotProduct, temp3);
        if (0b1111 & _mm_movemask_ps(_mm_cmpeq_ps(temp2, ARBOR getZeroVector())))
        {
            temp = ARBOR randomVector(1.0f);
            if (simd_cpu_capabilities::sse41())
            {
                temp2 = _mm_dp_ps(temp, temp, 0b00111111);
            }
            else
            {
                temp2 = _mm_mul_ps(temp, temp);
                temp2 = _mm_add_ps(temp2, _mm_shuffle_ps(temp2, temp2, 0b10110001));
            }
            temp2 = _mm_sqrt_ps(temp2);
        }
        temp = _mm_mul_ps(temp, _mm_rcp_ps(temp2));
        value.data[0] = repulsion;
        temp2 = _mm_load_ps(value.data);
        temp2 = _mm_shuffle_ps(temp2, temp2, 0);
        temp2 = _mm_mul_ps(m_mass, temp2);
        temp = _mm_mul_ps(temp, temp2);
        temp = _mm_mul_ps(temp, _mm_rcp_ps(temp3));
        v->applyForce(temp);
    }
}


/**
 * Finds out a quad in this branch where the specified particle is located.
 *
 * Parameters:
 * >p
 * Particle for which this method searches the quad.
 *
 * Returns:
 * Quad as `quad_index` enumeration type.
 *
 * Having result not equal `UnknownQuad` you can be sure that `p` has valid (non NaN) coordinates.
 *
 * Remarks:
 * This is `ArborGVT::BarnesHutTree::getQuad` method in the original C# code.
 */
quad_element::quad_index branch::getQuad(_In_ const particle* p) const noexcept
{
    quad_index result;
    __m128 coordinate = p->getCoordinates();
    if (0b0011 == (0b0011 & _mm_movemask_ps(_mm_cmpeq_ps(coordinate, coordinate))))
    {
        __m128 temp = _mm_shuffle_ps(m_area, m_area, 0b11111101);
        __m128 temp2 = _mm_sub_ps(coordinate, temp);
        temp = _mm_sub_ps(m_area, temp);
        sse_t value;
        value.data[0] = 0.5f;
        __m128 half = _mm_load_ps(value.data);
        half = _mm_shuffle_ps(half, half, 0);
        temp = _mm_mul_ps(temp, half);
        temp = _mm_shuffle_ps(temp, temp, 0b10001000);
        int compare = _mm_movemask_ps(_mm_cmplt_ps(temp2, temp));
        if (0b0001 & compare)
        {
            result = 0b0010 & compare ? NorthWestQuad : SouthWestQuad;
        }
        else
        {
            result = 0b0010 & compare ? NorthEastQuad : SouthEastQuad;
        }
    }
    else
    {
        result = UnknownQuad;
    }
    return result;
}


/**
 * Gets content of the specified quad.
 *
 * Parameters:
 * >quad
 * Index of the quad whose content is required.
 * >element
 * Pointer that receives a pointer to the quad element. If this method failed it receives `nullptr`.
 *
 * Returns:
 * If this method succeeded it returns `true` value and `false` otherwise. In the latter case caller should ignore value
 * of `*element` (it will be `nullptr`).
 */
_Check_return_ bool branch::getQuadContent(
    _In_ quad_index quad, _Outptr_result_maybenull_ quad_element** element) const noexcept
{
    if (UnknownQuad != quad)
    {
        *element = m_quads.at(quad).get();
        return true;
    }
    else
    {
        *element = nullptr;
        return false;
    }
}


/**
 * Increases parameters (mass and coordinates) of this branch using corresponding parameters of the specified particle.
 *
 * Parameters:
 * >p
 * Particle which parameters increase parameters of this branch. This branch should already or would contain the `p`.
 *
 * Returns:
 * N/A.
 */
void branch::increaseParameters(_In_ const particle* p) noexcept
{
    __m128 temp = p->getMass();
    m_mass = _mm_add_ps(m_mass, temp);
    temp = _mm_mul_ps(p->getCoordinates(), temp);
    m_coordinates = _mm_add_ps(m_coordinates, temp);
}


/**
 * Increases parameters of the specified current branch using the specified source particle.
 *
 * Parameters:
 * >p
 * Particle owned by this branch. Parameters of this particle affects parameters of this branch.
 * >b
 * The current branch.
 * >quad
 * Quad of the `b` where `p` is located.
 * >particles
 * Container with particles to be handled.
 *
 * Returns:
 * Pointer to a branch to be set as the current one.
 *
 * Remarks:
 * This is `ArborGVT::BarnesHutTree::insert` method in the original C# code.
 */
branch* particle::handleParticle(
    _In_ const particle* p, _In_ branch* b, _In_ quad_index quad, _In_ particles_cont_t* particles)
{
    __m128 temp = b->getArea();
    __m128 temp2 = _mm_shuffle_ps(temp, temp, 0b11111101);
    __m128 halfSize = _mm_sub_ps(temp, temp2);
    sse_t value;
    value.data[0] = 0.5f;
    temp2 = _mm_load_ps(value.data);
    temp2 = _mm_shuffle_ps(temp2, temp2, 0);
    halfSize = _mm_mul_ps(halfSize, temp2);
    __m128 origin = _mm_shuffle_ps(temp, temp, 0b10110001);
    temp2 = _mm_add_ps(origin, halfSize);
    if ((NorthEastQuad == quad) || (SouthEastQuad == quad))
    {
        temp = _mm_shuffle_ps(temp2, origin, 0b01100100);
        origin = _mm_shuffle_ps(temp, origin, 0b11101100);
    }
    if ((SouthWestQuad == quad) || (SouthEastQuad == quad))
    {
        temp = _mm_shuffle_ps(temp2, origin, 0b10111000);
        origin = _mm_shuffle_ps(origin, temp, 0b10010100);
    }
    temp = _mm_add_ps(origin, halfSize);
    temp = _mm_shuffle_ps(temp, origin, 0b10001000);
    temp = _mm_shuffle_ps(temp, temp, 0b11011000);
    auto newBranch = std::make_unique<branch>(temp);
    temp = p->getMass();
    b->setMass(temp);
    temp2 = p->getCoordinates();
    temp = _mm_mul_ps(temp, temp2);
    b->setCoordinates(temp);
    if (0b0011 == (0b0011 & _mm_movemask_ps(_mm_cmpeq_ps(m_vertex->getCoordinates(), temp2))))
    {
        value.data[0] = 0.08f;
        temp = _mm_load_ps(value.data);
        temp = _mm_shuffle_ps(temp, temp, 0);
        __m128 coefficient = _mm_mul_ps(halfSize, temp);
        coefficient = _mm_shuffle_ps(coefficient, coefficient, 0b10001000);
        temp = ARBOR randomVector(coefficient);
        temp = _mm_add_ps(m_vertex->getCoordinates(), temp);
        temp2 = _mm_shuffle_ps(origin, origin, 0b10001000);
        temp = _mm_max_ps(temp, temp2);
        temp2 = _mm_add_ps(origin, halfSize);
        temp2 = _mm_shuffle_ps(temp2, temp2, 0b10001000);
        m_vertex->setCoordinates(_mm_min_ps(temp, temp2));
    }
    particles->push_back(std::make_unique<particle>(m_vertex));
    branch* result = newBranch.get();
    // The following line definitely will `delete this`.
    b->setQuadContent(quad, std::move(newBranch));
    return result;
}


/**
 * Applies forces to Barnes Hut tree's element.
 *
 * Parmeters:
 * >v
 * Graph vertex.
 * >repulsion
 * Repulsion setting.
 * >dist
 * Another one setting.
 * >elements
 * Tree's elements to iterate over.
 *
 * Returns:
 * N/A.
 *
 * Remarks:
 * This is `ArborGVT::BarnesHutTree::applyForces` method in the original C# code.
 */
void particle::applyForce(
    _In_ ARBOR vertex* v,
    _In_ const float repulsion,
    _In_ const float,
    _In_ quad_elements_cont_t*) const
{
    __m128 temp = _mm_sub_ps(v->getCoordinates(), m_vertex->getCoordinates());
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
    __m128 dotProduct = temp2;
    temp2 = _mm_sqrt_ps(temp2);
    sse_t value;
    value.data[0] = 1.0f;
    __m128 temp3 = _mm_load_ps(value.data);
    temp3 = _mm_shuffle_ps(temp3, temp3, 0);
    temp3 = _mm_max_ps(dotProduct, temp3);
    if (0b1111 & _mm_movemask_ps(_mm_cmpeq_ps(temp2, ARBOR getZeroVector())))
    {
        temp = ARBOR randomVector(1.0f);
        if (simd_cpu_capabilities::sse41())
        {
            temp2 = _mm_dp_ps(temp, temp, 0b00111111);
        }
        else
        {
            temp2 = _mm_mul_ps(temp, temp);
            temp2 = _mm_add_ps(temp2, _mm_shuffle_ps(temp2, temp2, 0b10110001));
        }
        temp2 = _mm_sqrt_ps(temp2);
    }
    temp = _mm_mul_ps(temp, _mm_rcp_ps(temp2));
    value.data[0] = repulsion;
    temp2 = _mm_load_ps(value.data);
    temp2 = _mm_shuffle_ps(temp2, temp2, 0);
    temp2 = _mm_mul_ps(m_vertex->getMass(), temp2);
    temp = _mm_mul_ps(temp, temp2);
    temp = _mm_mul_ps(temp, _mm_rcp_ps(temp3));
    v->applyForce(temp);
}

BHUT_END
