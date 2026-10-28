#include "../../Speed/Indep/Libs/D3DX/EffectShaderResources.h"
#include "../../Speed/Indep/Libs/D3DX/EffectDebug.h"
#ifdef _MSC_VER
extern "C" __declspec(noreturn) void __cdecl abort(void);
#else
extern "C" void __cdecl abort(void) __attribute__((noreturn));
#endif

// Debug muting executes the recovered body and native module-absence path.
// GPU creation remains unsupported; activation must supply real shader objects.
extern "C" void __stdcall UG2FxDebugMute(unsigned int mute) { D3DXEffectDebugMute(mute); }
extern "C" int __stdcall UG2FxCreateVertexShader(unsigned int,unsigned int,const void *,unsigned int *) { abort(); }
extern "C" int __stdcall UG2FxCreatePixelShader(unsigned int,unsigned int,const void *,unsigned int *) { abort(); }
