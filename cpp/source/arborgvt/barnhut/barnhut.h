#pragma once
#include "barnhut\bhutquad.h"
#include "graph\vertex.h"
#include "ns\barnhut.h"
#include <memory>

BHUT_BEGIN

class barnes_hut_tree
{
public:
    barnes_hut_tree(_In_ __m128 area, _In_ const float dist)
        :
        m_root {std::make_unique<branch>(area)},
        m_dist {dist}
    {
    }

    void __fastcall insert(_In_ ARBOR vertex* v);


private:
    std::unique_ptr<branch> m_root;
    const float m_dist;
};

BHUT_END
