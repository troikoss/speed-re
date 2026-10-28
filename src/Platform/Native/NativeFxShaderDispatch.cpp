#include "../../Speed/Indep/Libs/D3DX/EffectShaderResources.h"
#ifdef _MSC_VER
extern "C" __declspec(noreturn) void __cdecl abort(void);
#else
extern "C" void __cdecl abort(void) __attribute__((noreturn));
#endif

// These library/debug and GPU boundaries remain unsupported. Real effect
// activation must supply them; no successful shader object is fabricated.
extern "C" void __stdcall UG2FxDebugMute(unsigned int) { abort(); }
extern "C" int __stdcall UG2FxCreateVertexShader(unsigned int,unsigned int,const void *,unsigned int *) { abort(); }
extern "C" int __stdcall UG2FxCreatePixelShader(unsigned int,unsigned int,const void *,unsigned int *) { abort(); }
