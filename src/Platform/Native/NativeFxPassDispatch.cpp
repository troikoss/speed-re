#include "../../Speed/Indep/Libs/D3DX/EffectPassRuntime.h"
#include "../../Speed/Indep/Libs/D3DX/EffectStateCache.h"
#include <stdio.h>
#include <stdlib.h>
// Cache control reaches recovered source. The remaining record/group dispatcher
// bodies stay separate; probe fixtures can override these typed boundaries.
namespace {
__attribute__((noreturn)) void Unsupported(const char *address) {
    fprintf(stderr,"unimplemented effect state dispatch: %s\n",address);abort();
}
}
extern "C" __attribute__((weak)) int UG2FxPassCache66403C(D3DXEffectObject *e,int enabled,void *sizes) { return e->CacheVertexState(enabled,static_cast<unsigned short *>(sizes)); }
extern "C" __attribute__((weak)) int UG2FxPassCache664197(D3DXEffectObject *e,int enabled,void *sizes) { return e->CachePixelState(enabled,static_cast<unsigned short *>(sizes)); }
extern "C" __attribute__((weak)) int UG2FxPassFastGroup668DFD(D3DXEffectObject *,D3DXEffectGroup64 *) { Unsupported("0x668dfd"); }
extern "C" __attribute__((weak)) int UG2FxPassRecord669286(D3DXEffectObject *,unsigned int,unsigned int,unsigned int) { Unsupported("0x669286"); }
extern "C" __attribute__((weak)) int UG2FxPassCachedRecord66A25F(D3DXEffectObject *,D3DXEffectGroup64 *,D3DXEffectRecord64 *) { Unsupported("0x66a25f"); }
