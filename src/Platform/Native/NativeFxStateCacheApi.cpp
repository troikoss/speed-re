#include "../../Speed/Indep/Libs/D3DX/EffectStateCache.h"
#include <stdio.h>
#include <stdlib.h>
// The original state-block and shader/sampler device methods require native
// GPU objects. Keep these typed boundaries visible until that integration.
namespace {
__attribute__((noreturn)) void Unsupported(const char *name) { fprintf(stderr,"unimplemented state-cache GPU API: %s\n",name);abort(); }
}
extern "C" __attribute__((weak)) int UG2FxStateBlockBegin(unsigned int) { Unsupported("BeginStateBlock"); }
extern "C" __attribute__((weak)) int UG2FxStateBlockEnd(unsigned int,unsigned int *) { Unsupported("EndStateBlock"); }
extern "C" __attribute__((weak)) int UG2FxStateBlockCapture(unsigned int) { Unsupported("Capture"); }
extern "C" __attribute__((weak)) int UG2FxStateBlockApply(unsigned int) { Unsupported("Apply"); }
extern "C" __attribute__((weak)) unsigned int UG2FxStateBlockRelease(unsigned int) { Unsupported("Release"); }
extern "C" __attribute__((weak)) int UG2FxCacheSetVertexShader(unsigned int,unsigned int) { Unsupported("SetVertexShader"); }
extern "C" __attribute__((weak)) int UG2FxCacheSetPixelShader(unsigned int,unsigned int) { Unsupported("SetPixelShader"); }
extern "C" __attribute__((weak)) int UG2FxCacheVertexFloat(unsigned int,unsigned int,const void *,unsigned int) { Unsupported("VertexFloat"); }
extern "C" __attribute__((weak)) int UG2FxCacheVertexInt(unsigned int,unsigned int,const void *,unsigned int) { Unsupported("VertexInt"); }
extern "C" __attribute__((weak)) int UG2FxCacheVertexBool(unsigned int,unsigned int,const void *,unsigned int) { Unsupported("VertexBool"); }
extern "C" __attribute__((weak)) int UG2FxCachePixelFloat(unsigned int,unsigned int,const void *,unsigned int) { Unsupported("PixelFloat"); }
extern "C" __attribute__((weak)) int UG2FxCachePixelInt(unsigned int,unsigned int,const void *,unsigned int) { Unsupported("PixelInt"); }
extern "C" __attribute__((weak)) int UG2FxCachePixelBool(unsigned int,unsigned int,const void *,unsigned int) { Unsupported("PixelBool"); }
extern "C" __attribute__((weak)) int UG2FxCacheSetTexture(unsigned int,unsigned int,unsigned int) { Unsupported("SetTexture"); }
extern "C" __attribute__((weak)) int UG2FxCacheSetSampler(unsigned int,unsigned int,unsigned int,unsigned int) { Unsupported("SetSamplerState"); }
