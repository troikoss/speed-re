#include "../../Speed/Indep/Libs/D3DX/EffectHandles.h"
#ifdef _MSC_VER
extern "C" __declspec(noreturn) void __cdecl abort(void);
#else
extern "C" void __cdecl abort(void) __attribute__((noreturn));
#endif

// Platform scaffolding for the initial, immutable UG2 C-locale context. The
// recovered decimal body reads the original mapped locale/ctype data. Dynamic
// CRT locale changes and multibyte classification remain unsupported.
extern "C" const void * __cdecl UG2FxCrtGetThreadData() {
    static const struct { unsigned int prefix[25];unsigned int locale64; } context={{0},0x819cd0};
    return &context;
}
extern "C" const void * __cdecl UG2FxCrtUpdateLocale() { abort(); }
extern "C" unsigned int __cdecl UG2FxCrtClassify(const void *,unsigned int,unsigned int) { abort(); }
