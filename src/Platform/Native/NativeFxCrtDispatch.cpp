#include "../../Speed/Indep/Libs/D3DX/CrtExceptions.h"
#ifdef _MSC_VER
extern "C" __declspec(noreturn) void __cdecl abort(void);
#else
extern "C" void __cdecl abort(void) __attribute__((noreturn));
#endif
// The original default callback is recovered. Other PE function tokens need
// a separately evidenced native implementation before they can be dispatched.
extern "C" int __cdecl UG2FxCrtCallMathError(unsigned int token,D3DXCrtMathException *error) {
    if (token==0x727e6b) return D3DXCrtDefaultMathError(error);
    abort();
}
// Preserve the caller's raw CDECL payload before the recovered body chooses
// which values to load. In particular, unused SNaNs must not be quieted here.
extern "C" long double __cdecl UG2FxCrtHandleResult(unsigned int flags,unsigned int operation,D3DXCrtDoubleBits first,D3DXCrtDoubleBits result,unsigned int control) {
    return D3DXCrtHandleResult(flags,operation,first,result,control);
}
