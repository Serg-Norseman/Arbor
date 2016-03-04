#pragma once
#include "dlllayer\arborvis.h"

#if defined(ARBORGVT_EXPORTS)
#define ARBORGVT_API __declspec(dllexport)
#else
#define ARBORGVT_API __declspec(dllimport)
#endif

ARBORGVT_API HRESULT __stdcall createArborVisual(_Outptr_result_maybenull_ IArborVisual** visual);
