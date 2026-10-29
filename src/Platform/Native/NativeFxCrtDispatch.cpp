#include "../../Speed/Indep/Libs/D3DX/CrtFloor.h"
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
// Original general FP exception/result runtime 0x7675cb remains unsupported.
extern "C" long double __cdecl UG2FxCrtHandleResult(unsigned int,unsigned int,double,double,unsigned int) { abort(); }
