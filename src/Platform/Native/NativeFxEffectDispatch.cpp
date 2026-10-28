#include "../../Speed/Indep/Libs/D3DX/EffectFactory.h"
extern "C" void __cdecl abort(void);

// Native ABI bridges for the two known class tokens. The effect factory is
// not activated until compiler/runtime preparation and GPU support exist.
extern "C" unsigned int UG2FxEffectAddRefVirtual(D3DXEffectObject *effect) {
    if (effect->class00!=0x7b6fe0) abort();
    return D3DXEffectAddRef(effect);
}
extern "C" unsigned int UG2FxEffectReleaseVirtual(D3DXEffectObject *effect) {
    if (effect->class00!=0x7b6fe0) abort();
    return D3DXEffectRelease(effect);
}
extern "C" unsigned char *UG2FxBufferPointerVirtual(D3DXEffectBuffer *buffer) {
    if (buffer->class00!=0x7b711c) abort();
    return D3DXEffectBufferPointer(buffer);
}
extern "C" unsigned int UG2FxBufferLengthVirtual(D3DXEffectBuffer *buffer) {
    if (buffer->class00!=0x7b711c) abort();
    return D3DXEffectBufferLength(buffer);
}
extern "C" unsigned int UG2FxBufferReleaseVirtual(D3DXEffectBuffer *buffer) {
    if (buffer->class00!=0x7b711c) abort();
    return D3DXEffectBufferRelease(buffer);
}
