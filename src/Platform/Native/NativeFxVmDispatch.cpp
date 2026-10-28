#include "../../Speed/Indep/Libs/D3DX/EffectEvaluation.h"
#ifdef _MSC_VER
extern "C" __declspec(noreturn) void __cdecl abort(void);
#else
extern "C" void __cdecl abort(void) __attribute__((noreturn));
#endif
// VM 0x688e79 remains unrecovered. Do not substitute host math or a successful
// shader result. Comparative verification executes the original VM separately.
extern "C" int __stdcall UG2FxExecuteVm(unsigned int *,unsigned int *,unsigned int,unsigned int,unsigned int,
    unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int) { abort(); }
