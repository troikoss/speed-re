#include "../../Speed/Indep/Libs/D3DX/EffectVm.h"
#ifdef _MSC_VER
extern "C" __declspec(noreturn) void __cdecl abort(void);
#else
extern "C" void __cdecl abort(void) __attribute__((noreturn));
#endif
// Four unresolved CRT dependencies of the recovered VM dispatch. Do not
// substitute host CRT approximations for the original runtime.
extern "C" long double __cdecl UG2FxVmFloor(double) { abort(); }
extern "C" long double __cdecl UG2FxVmPower(double,double) { abort(); }
extern "C" long double __cdecl UG2FxVmAsin(double) { abort(); }
extern "C" long double __cdecl UG2FxVmAcos(double) { abort(); }
// Preserve the recovered operation's extended x87 return through this bridge.
extern "C" long double __stdcall UG2FxVmOperation63F2B5(float *values) { return D3DXEffectVmOperation63F2B5(values); }
