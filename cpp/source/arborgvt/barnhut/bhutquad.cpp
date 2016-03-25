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
 * >v
 * Graph vertex. This is the vertex `p` is based on.
 *
 * Returns:
 * Pointer to a branch to be set as the current one.
 *
 * Remarks:
 * This is `ArborGVT::BarnesHutTree::insert` method in the original C# code.
 */
branch* branch::handleParticle(
    _In_ const particle* p, _In_ branch* b, _In_ quad_index, _In_ particles_cont_t*, _In_ ARBOR vertex*)
{
    b->increaseParameters(p);
    return this;
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
    sse_t value;
    value.data[0] = p->getMass();
    m_mass += value.data[0];
    __m128 temp = _mm_load_ps(value.data);
    temp = _mm_shuffle_ps(temp, temp, 0);
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
 * >v
 * Graph vertex. This is the vertex `p` is based on.
 *
 * Returns:
 * Pointer to a branch to be set as the current one.
 *
 * Remarks:
 * This is `ArborGVT::BarnesHutTree::insert` method in the original C# code.
 */
branch* particle::handleParticle(
    _In_ const particle* p,
    _In_ branch* b,
    _In_ quad_index quad,
    _In_ particles_cont_t* particles,
    _In_ ARBOR vertex* v)
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
    value.data[0] = p->getMass();
    b->setMass(value.data[0]);
    temp = _mm_load_ps(value.data);
    temp = _mm_shuffle_ps(temp, temp, 0);
    temp2 = p->getCoordinates();
    temp = _mm_mul_ps(temp2, temp);
    b->setCoordinates(temp);
    if (0b0011 == (0b0011 & _mm_movemask_ps(_mm_cmpeq_ps(m_coordinates, temp2))))
    {
        value.data[0] = 0.08f;
        temp = _mm_load_ps(value.data);
        temp = _mm_shuffle_ps(temp, temp, 0);
        __m128 coefficient = _mm_mul_ps(halfSize, temp);
        coefficient = _mm_shuffle_ps(coefficient, coefficient, 0b10001000);
        temp = ARBOR randomVector(coefficient);
        temp = _mm_add_ps(m_coordinates, temp);
        temp2 = _mm_shuffle_ps(origin, origin, 0b10001000);
        temp = _mm_max_ps(temp, temp2);
        temp2 = _mm_add_ps(origin, halfSize);
        temp2 = _mm_shuffle_ps(temp2, temp2, 0b10001000);
        m_coordinates = _mm_min_ps(temp, temp2);
        v->setCoordinates(m_coordinates);
    }
    particles->push_back(std::make_unique<particle>(m_coordinates, m_mass));
    branch* result = newBranch.get();
    // The following line definitely will `delete this`.
    b->setQuadContent(quad, std::move(newBranch));
    return result;
}

BHUT_END
