#include "../../Speed/Indep/Libs/D3DX/EffectVm.h"
#ifdef _MSC_VER
extern "C" __declspec(noreturn) void __cdecl abort(void);
#else
extern "C" void __cdecl abort(void) __attribute__((noreturn));
#endif
// Explicit unresolved dependencies of the recovered VM dispatch. Do not
// substitute host CRT approximations for the original runtime/operation.
extern "C" long double __cdecl UG2FxVmFloor(double) { abort(); }
extern "C" long double __cdecl UG2FxVmPower(double,double) { abort(); }
extern "C" long double __cdecl UG2FxVmAsin(double) { abort(); }
extern "C" long double __cdecl UG2FxVmAcos(double) { abort(); }
extern "C" long double __stdcall UG2FxVmOperation63F2B5(float *) { abort(); }
