#include "../../Speed/Indep/Libs/D3DX/EffectValues.h"
extern "C" void __cdecl abort(void);
// Original relocation calls effect vtable+0x50. Native tokens are data only.
extern "C" int UG2FxSetValueVirtual(D3DXEffectObject *effect,unsigned int handle,const void *data,unsigned int bytes) {
    if (effect->class00!=0x7b6fe0) abort();
    return D3DXEffectSetValue(effect,handle,data,bytes);
}
