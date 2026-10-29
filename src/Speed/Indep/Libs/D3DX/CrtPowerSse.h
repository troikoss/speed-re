#ifndef UG2_D3DX_CRT_POWER_SSE_H
#define UG2_D3DX_CRT_POWER_SSE_H
#include "CrtPower.h"
// Algorithm values, not physical-register bindings. Keep both lanes because
// packed arithmetic and integer predicates are observable in the original.
struct D3DXCrtSseValue { unsigned int word[4]; };
typedef char D3DXCrtSseValueSize[(sizeof(D3DXCrtSseValue)==16)?1:-1];
extern "C" {
void __cdecl D3DXCrtPowerSseResult(double *,D3DXCrtDoubleBits,D3DXCrtDoubleBits); // 0x764749, final FLD represented by its caller
void __cdecl D3DXCrtPowerSse80(D3DXCrtExtended *,D3DXCrtDoubleBits,D3DXCrtDoubleBits); // full extended-return adapter
void __cdecl UG2FxCrtSseAddScalar(D3DXCrtSseValue *,const D3DXCrtSseValue *);
void __cdecl UG2FxCrtSseSubtractScalar(D3DXCrtSseValue *,const D3DXCrtSseValue *);
void __cdecl UG2FxCrtSseMultiplyScalar(D3DXCrtSseValue *,const D3DXCrtSseValue *);
void __cdecl UG2FxCrtSseDivideScalar(D3DXCrtSseValue *,const D3DXCrtSseValue *);
void __cdecl UG2FxCrtSseAddPacked(D3DXCrtSseValue *,const D3DXCrtSseValue *);
void __cdecl UG2FxCrtSseMultiplyPacked(D3DXCrtSseValue *,const D3DXCrtSseValue *);
void __cdecl UG2FxCrtSseConvertDwords(D3DXCrtSseValue *);
unsigned int __cdecl UG2FxCrtSseConvertInteger(const D3DXCrtSseValue *);
extern const unsigned char ug2CrtPowerSseData7D4310[0x3928];
}
#endif
