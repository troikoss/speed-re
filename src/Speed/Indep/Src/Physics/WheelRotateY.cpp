#include "WheelMath.h"
#include "EngineAngleMath.h"
extern "C" WheelMatrix *__stdcall D3DXMatrixMultiply(WheelMatrix *,WheelMatrix *,WheelMatrix *);

// Original complete 0x5bada0..0x5bae50. Cosine is narrowed before the sine
// call. Negative and positive sine are narrowed independently from its x87
// return, preserving directed rounding. The private product thunk selects the
// original scalar implementation in the native ABI bridge.
void RotateMatrixY(WheelMatrix *output,WheelMatrix *input,unsigned int angle) {
    volatile float cosine=EngineCosineUnitsExtended(angle);
    volatile long double sine=EngineSineUnitsExtended(angle);
    WheelMatrix rotation;float *m=rotation.elements;
    volatile long double negativeSine=-sine;
    m[2]=negativeSine;m[8]=sine;m[0]=cosine;m[10]=cosine;
    m[1]=m[3]=m[4]=m[6]=m[7]=m[9]=m[11]=m[12]=m[13]=m[14]=0;
    m[5]=m[15]=1;
    D3DXMatrixMultiply(output,input,&rotation);
}
