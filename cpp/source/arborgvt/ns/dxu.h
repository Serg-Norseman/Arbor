#pragma once
/*
 * Macros, that define/use the 'dxu' namespace (DirectX util).
 * Can't use 'DX' shortcut -- there's 'DX' field name in the 'MilMatrix3x2D' structure (@Windows API's dwmapi.h).
 */
#define DXU_BEGIN namespace dxu {
#define DXU_END }
#define DXU ::dxu::
