#ifndef UG2_D3DX_CRT_POWER_H
#define UG2_D3DX_CRT_POWER_H
#include "CrtInverse.h"
struct D3DXCrtSavedEnvironment { unsigned char bytes[108]; };
extern "C" {
void __cdecl D3DXCrtPower80(D3DXCrtExtended *,const D3DXCrtExtended *,const D3DXCrtExtended *); // 0x75c9b0
void __cdecl D3DXCrtPowerX8780(D3DXCrtExtended *,D3DXCrtDoubleBits,D3DXCrtDoubleBits,const D3DXCrtExtended *); // 0x75ca0d
void __cdecl D3DXCrtPowerSseWrapper80(D3DXCrtExtended *,const D3DXCrtExtended *,const D3DXCrtExtended *); // 0x764730
unsigned int __cdecl D3DXCrtPowerLoadOperand(D3DXCrtExtended *,D3DXCrtDoubleBits,unsigned int *); // 0x765705, EAX/ZF plus ECX side effect
void __cdecl D3DXCrtReconstructExp2(D3DXCrtExtended *); // 0x7656c0
unsigned int __cdecl D3DXCrtPowerParity80(const D3DXCrtExtended *); // 0x75cbd2, consumed ST0 / CL result
int __cdecl D3DXCrtPowerParity(D3DXCrtDoubleBits); // 0x76584c
int __cdecl D3DXCrtFpClass(D3DXCrtDoubleBits); // 0x75dc8e
int __cdecl D3DXCrtPowerSpecial(D3DXCrtDoubleBits,D3DXCrtDoubleBits,double *); // 0x7658b6
void __cdecl D3DXCrtTwoArgError(unsigned int,unsigned int,const char *,D3DXCrtDoubleBits,D3DXCrtDoubleBits,unsigned int,D3DXCrtExtended *); // 0x765660 + shared tail 0x765680
void __cdecl D3DXCrtPowerMathExit(unsigned int,const char *,D3DXCrtDoubleBits,D3DXCrtDoubleBits,unsigned int,D3DXCrtExtended *); // 0x7657a9
// Instruction semantics and the remaining original SSE body boundary.
void __cdecl UG2FxCrtExtendedLoad(D3DXCrtExtended *,const D3DXCrtExtended *);
void __cdecl UG2FxCrtExtendedAdd(D3DXCrtExtended *,const D3DXCrtExtended *,const D3DXCrtExtended *);
void __cdecl UG2FxCrtExtendedRound(D3DXCrtExtended *,const D3DXCrtExtended *);
void __cdecl UG2FxCrtExtendedSubtract(D3DXCrtExtended *,const D3DXCrtExtended *,const D3DXCrtExtended *);
void __cdecl UG2FxCrtExtendedNegate(D3DXCrtExtended *);
void __cdecl UG2FxCrtExtendedAbs(D3DXCrtExtended *,const D3DXCrtExtended *);
void __cdecl UG2FxCrtExtendedF2xm1(D3DXCrtExtended *);
void __cdecl UG2FxCrtExtendedScale(D3DXCrtExtended *,const D3DXCrtExtended *);
void __cdecl UG2FxCrtExtendedFyl2x(D3DXCrtExtended *,const D3DXCrtExtended *,const D3DXCrtExtended *);
void __cdecl UG2FxCrtExtendedMultiplyDouble(D3DXCrtExtended *,const double *);
unsigned int __cdecl UG2FxCrtExtendedCompare(const D3DXCrtExtended *,const D3DXCrtExtended *);
unsigned int __cdecl UG2FxCrtExtendedCompareDouble(const D3DXCrtExtended *,const double *);
void __cdecl UG2FxCrtSaveEnvironment(D3DXCrtSavedEnvironment *);
void __cdecl UG2FxCrtRestoreEnvironment(const D3DXCrtSavedEnvironment *);
void __cdecl UG2FxCrtPowerSseBody(double *,D3DXCrtDoubleBits,D3DXCrtDoubleBits); // 0x764749 remains unsupported
extern const double ug2CrtPowerHalf819C80,ug2CrtPowerIndefinite81A818,ug2CrtPowerNegativeZero81A830;
extern const double ug2CrtPowerMaximum7D7C54,ug2CrtPowerMinimum7D7C5C,ug2CrtPowerScaleDown7D7C64,ug2CrtPowerScaleUp7D7C6C,ug2CrtPowerInfinity7D7C74,ug2CrtPowerZero7D7C7C;
extern const D3DXCrtExtended ug2CrtPowerInfinity81A470;
}
#endif
