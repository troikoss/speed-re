#ifndef UG2_D3DX_MATRIX_SCALAR_H
#define UG2_D3DX_MATRIX_SCALAR_H
#include "WheelMath.h"
// Scalar library body 0x63cee1; the original thunk 0x63cedb selects a runtime
// implementation. Other optimized implementations are not represented here.
WheelMatrix *__stdcall D3DXMatrixMultiplyScalar(WheelMatrix *output,const WheelMatrix *left,const WheelMatrix *right);
#endif
