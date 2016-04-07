#include "barnhut\barnhut.h"
#include <memory>

BHUT_BEGIN

/**
 * Treats the specified graph vertex as a particle and place it to this Barnes Hut tree.
 *
 * Parmeters:
 * >v
 * Graph vertex.
 *
 * Returns:
 * N/A.
 *
 * Remarks:
 * This is `ArborGVT::BarnesHutTree::insert` method in the original C# code.
 */
void barnes_hut_tree::insert(_In_ ARBOR vertex* v)
{
    branch* currentBranch = m_root.get();
    auto currentParticle = std::make_unique<particle>(v);
    quad_element::particles_cont_t particles {};
    while (currentParticle || particles.size())
    {
        if (!currentParticle)
        {
            currentParticle = std::move(particles.front());
            particles.pop_front();
        }

        branch::quad_index quad = currentBranch->getQuad(currentParticle.get());
        quad_element* quadElement;
        if (currentBranch->getQuadContent(quad, &quadElement))
        {
            if (nullptr == quadElement)
            {
                currentBranch->increaseParameters(currentParticle.get());
                currentBranch->setQuadContent(quad, std::move(currentParticle));
            }
            else
            {
                currentBranch = quadElement->handleParticle(currentParticle.get(), currentBranch, quad, &particles);
            }
        }
    }
}


/**
 * Applies forces to each particle in this Barnes Hut tree.
 *
 * Parmeters:
 * >v
 * Graph vertex.
 * >repulsion
 * Repulsion setting.
 *
 * Returns:
 * N/A.
 *
 * Remarks:
 * This is `ArborGVT::BarnesHutTree::applyForces` method in the original C# code.
 */
void barnes_hut_tree::applyForce(_In_ ARBOR vertex* v, _In_ const float repulsion) const
{
    quad_element::quad_elements_cont_t elements {1};
    elements.front() = m_root.get();
    while (elements.size())
    {
        const quad_element* element = elements.front();
        elements.pop_front();
        if (element && (v != element->getVertex()))
        {
            element->applyForce(v, repulsion, m_dist, &elements);
        }
    }
}

BHUT_END
