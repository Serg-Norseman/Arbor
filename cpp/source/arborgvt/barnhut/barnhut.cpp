#include "barnhut\barnhut.h"
#include <memory>

BHUT_BEGIN

/**
 * Treats the specified graph vertex as a particle and place it in Barnes Hut tree.
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
    auto currentParticle = std::make_unique<particle>(v->getCoordinates(), v->getMass());
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
                currentBranch = quadElement->handleParticle(currentParticle.get(), currentBranch, quad, &particles, v);
            }
        }
    }
}

BHUT_END
