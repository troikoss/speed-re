#include "EffectVm.h"
#ifdef _MSC_VER
typedef __int64 OperationInteger;
#else
typedef long long OperationInteger;
#endif
extern "C" {
extern const float ug2VmOperationOne784250,ug2VmOperationBias784500;
extern const float ug2VmOperationThree787ADC,ug2VmOperationScale785110;
extern const unsigned int ug2VmOperationPermutation7A9460[514];
extern const float ug2VmOperationGradients7A9C68[514][3];
}

// Original 0x63f2b5..0x63f51f, stdcall pointer argument / RET 4.
// Keep the original z/y/x product order, explicitly narrowed intermediates,
// and the extended x-minus-one value used before its final narrowed reload.
extern "C" long double __stdcall D3DXEffectVmOperation63F2B5(float *values) {
    const long double sumX=static_cast<long double>(values[0])+ug2VmOperationBias784500;
    const int integerX=static_cast<int>(static_cast<OperationInteger>(sumX));
    const long double x=sumX-integerX,xMinus=x-ug2VmOperationOne784250;
    volatile float xMinusStored=xMinus;
    const long double sumY=static_cast<long double>(values[1])+ug2VmOperationBias784500;
    const int integerY=static_cast<int>(static_cast<OperationInteger>(sumY));
    const long double y=sumY-integerY;
    volatile float yMinus=y-ug2VmOperationOne784250;
    const long double sumZ=static_cast<long double>(values[2])+ug2VmOperationBias784500;
    const int integerZ=static_cast<int>(static_cast<OperationInteger>(sumZ));
    const long double z=sumZ-integerZ;
    volatile float zStored=z,zMinus=z-ug2VmOperationOne784250;
    const unsigned int ix=static_cast<unsigned int>(integerX)&255,iy=static_cast<unsigned int>(integerY)&255,iz=static_cast<unsigned int>(integerZ)&255;
    const unsigned int nextX=(ix+1)&255,nextY=(iy+1)&255,nextZ=(iz+1)&255;
    const unsigned int p0=ug2VmOperationPermutation7A9460[ix],p1=ug2VmOperationPermutation7A9460[nextX];
    const unsigned int p00=ug2VmOperationPermutation7A9460[p0+iy],p10=ug2VmOperationPermutation7A9460[p1+iy];
    const unsigned int p01=ug2VmOperationPermutation7A9460[p0+nextY],p11=ug2VmOperationPermutation7A9460[p1+nextY];
    volatile float weightX=((ug2VmOperationThree787ADC-(x+x))*x)*x;
    volatile float weightY=((ug2VmOperationThree787ADC-(y+y))*y)*y;
    const float *g=ug2VmOperationGradients7A9C68[p00+iz];
    const long double dot000=(z*g[2]+y*g[1])+x*g[0];
    g=ug2VmOperationGradients7A9C68[p10+iz];
    const long double dot100=(z*g[2]+y*g[1])+xMinus*g[0];
    const long double row00=(dot100-dot000)*weightX+dot000;
    g=ug2VmOperationGradients7A9C68[p01+iz];
    const long double dot010=(z*g[2]+static_cast<long double>(yMinus)*g[1])+x*g[0];
    g=ug2VmOperationGradients7A9C68[p11+iz];
    const long double dot110=(z*g[2]+static_cast<long double>(yMinus)*g[1])+xMinus*g[0];
    volatile float plane0=(((dot110-dot010)*weightX+dot010)-row00)*weightY+row00;
    g=ug2VmOperationGradients7A9C68[p00+nextZ];
    const long double dot001=(static_cast<long double>(zMinus)*g[2]+y*g[1])+x*g[0];
    g=ug2VmOperationGradients7A9C68[p10+nextZ];
    const long double dot101=(static_cast<long double>(zMinus)*g[2]+y*g[1])+xMinus*g[0];
    volatile float row01=(dot101-dot001)*weightX+dot001;
    g=ug2VmOperationGradients7A9C68[p01+nextZ];
    const long double dot011=(static_cast<long double>(zMinus)*g[2]+static_cast<long double>(yMinus)*g[1])+x*g[0];
    g=ug2VmOperationGradients7A9C68[p11+nextZ];
    const long double dot111=(static_cast<long double>(zMinus)*g[2]+static_cast<long double>(yMinus)*g[1])+static_cast<long double>(xMinusStored)*g[0];
    const long double difference=(((dot111-dot011)*weightX+dot011)-row01)*weightY+row01-plane0;
    return ((((ug2VmOperationThree787ADC-(static_cast<long double>(zStored)+zStored))*zStored)*zStored)*difference+plane0)*ug2VmOperationScale785110;
}
