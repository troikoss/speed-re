#include "../../Speed/Indep/Src/Physics/D3DXMatrixScalar.h"
// Native ABI selection of the recovered default scalar D3DX bodies. Original
// optimized dispatch/CPU initialization remains separate from this bridge.
extern "C" WheelMatrix *__stdcall D3DXMatrixMultiply(WheelMatrix *output,WheelMatrix *left,WheelMatrix *right) {
    return D3DXMatrixMultiplyScalar(output,left,right);
}
extern "C" WheelMatrix *__stdcall D3DXMatrixTranspose(WheelMatrix *output,WheelMatrix *input) {
    return D3DXMatrixTransposeScalar(output,input);
}
