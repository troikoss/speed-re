#include "../../Speed/Indep/Libs/D3DX/EffectVm.h"
#include "../../Speed/Indep/Libs/D3DX/CrtPower.h"
// Recovered math/operation bridges. Both x87 and SSE power execute recovered
// bodies; OS exception delivery and CRT startup remain platform boundaries.
extern "C" long double __cdecl UG2FxVmFloor(double value) { return D3DXCrtFloor(value); }
extern "C" long double __cdecl UG2FxVmPower(double base,double exponent) { D3DXCrtExtended a,b,result;UG2FxCrtExtendedFromDouble(&a,&base);UG2FxCrtExtendedFromDouble(&b,&exponent);D3DXCrtPower80(&result,&a,&b);return UG2FxCrtReturnExtended(&result); }
extern "C" long double __cdecl UG2FxVmAsin(double value) { D3DXCrtExtended input,result;UG2FxCrtExtendedFromDouble(&input,&value);D3DXCrtAsin80(&result,&input);return UG2FxCrtReturnExtended(&result); }
extern "C" long double __cdecl UG2FxVmAcos(double value) { D3DXCrtExtended input,result;UG2FxCrtExtendedFromDouble(&input,&value);D3DXCrtAcos80(&result,&input);return UG2FxCrtReturnExtended(&result); }
// Preserve the recovered operation's extended x87 return through this bridge.
extern "C" long double __stdcall UG2FxVmOperation63F2B5(float *values) { return D3DXEffectVmOperation63F2B5(values); }
