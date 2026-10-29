#include "../../Speed/Indep/Libs/D3DX/CrtPower.h"
#ifdef _MSC_VER
extern "C" __declspec(noreturn) void __cdecl abort(void);
#else
extern "C" void __cdecl abort(void) __attribute__((noreturn));
#endif
// Original SSE power body 0x764749 is not yet recovered. Production retains
// the original zero BSS CPU flag until CRT startup is recovered; explicitly
// enabled unsupported SSE dispatch remains visible as a trap.
extern "C" void __cdecl UG2FxCrtPowerSseBody(double *,D3DXCrtDoubleBits,D3DXCrtDoubleBits) { abort(); }
