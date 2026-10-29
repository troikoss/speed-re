#include "../../Speed/Indep/Libs/D3DX/EffectVm.h"
#include "../../Speed/Indep/Libs/D3DX/CrtInverse.h"
#ifdef _MSC_VER
extern "C" __declspec(noreturn) void __cdecl abort(void);
#else
extern "C" void __cdecl abort(void) __attribute__((noreturn));
#endif
// Recovered floor, inverse-trig and table-operation bridges. Power stays a trap;
// shared FP handling is recovered through the actual OS exception boundary.
extern "C" long double __cdecl UG2FxVmFloor(double value) { return D3DXCrtFloor(value); }
extern "C" long double __cdecl UG2FxVmPower(double,double) { abort(); }
extern "C" long double __cdecl UG2FxVmAsin(double value) { D3DXCrtExtended input,result;UG2FxCrtExtendedFromDouble(&input,&value);D3DXCrtAsin80(&result,&input);return UG2FxCrtReturnExtended(&result); }
extern "C" long double __cdecl UG2FxVmAcos(double value) { D3DXCrtExtended input,result;UG2FxCrtExtendedFromDouble(&input,&value);D3DXCrtAcos80(&result,&input);return UG2FxCrtReturnExtended(&result); }
// Preserve the recovered operation's extended x87 return through this bridge.
extern "C" long double __stdcall UG2FxVmOperation63F2B5(float *values) { return D3DXEffectVmOperation63F2B5(values); }
