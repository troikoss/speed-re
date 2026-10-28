#include "../../Speed/Indep/Libs/D3DX/EffectDestruction.h"
extern "C" void __cdecl abort(void);

extern "C" unsigned int __stdcall UG2FxComAddRef(unsigned int object) {
    switch (*reinterpret_cast<const unsigned int *>(object)) {
    case 0x7b711c:
    case 0x7ab4e8: return D3DXEffectBufferAddRef(reinterpret_cast<D3DXEffectBuffer *>(object));
    case 0x7b6fe0: return D3DXEffectAddRef(reinterpret_cast<D3DXEffectObject *>(object));
    default: abort();
    }
}

// Original ownership calls COM vtable+4/+8. Native class tokens cover only the
// recovered CPU buffer/pool/effect objects. Device/shader classes remain open.
extern "C" unsigned int __stdcall UG2FxComRelease(unsigned int object) {
    switch (*reinterpret_cast<const unsigned int *>(object)) {
    case 0x7b711c: return D3DXEffectBufferRelease(reinterpret_cast<D3DXEffectBuffer *>(object));
    case 0x7ab4e8: return D3DXEffectPoolRelease(reinterpret_cast<D3DXEffectPool *>(object));
    case 0x7b6fe0: return D3DXEffectRelease(reinterpret_cast<D3DXEffectObject *>(object));
    default: abort();
    }
}
