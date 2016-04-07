#include "dlllayer\arbor.h"
#include "ui\nowindow\avisimpl\avisimpl.h"

/**
 * Creates `IArborVisual` object used to control graph representation and rendering.
 */
ARBORGVT_API HRESULT __stdcall createArborVisual(_Outptr_result_maybenull_ IArborVisual** visual)
{
    *visual = new arbor_visual_impl {};
    return S_OK;
}
