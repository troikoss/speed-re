#include "../../Speed/Indep/Libs/D3DX/EffectFactory.h"
extern "C" void __cdecl abort(void);

// Pool +4 is the same reference-count member used by original entry 0x670dde.
extern "C" unsigned int UG2FxPoolAddRefVirtual(D3DXEffectPool *pool) {
    if (pool->class00!=0x7ab4e8) abort();
    return D3DXEffectBufferAddRef(reinterpret_cast<D3DXEffectBuffer *>(pool));
}
