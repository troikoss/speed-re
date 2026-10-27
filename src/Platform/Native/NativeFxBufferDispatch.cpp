#include "../../Speed/Indep/Libs/D3DX/EffectInput.h"
extern "C" void __cdecl abort(void);

namespace {
void Check(D3DXEffectBuffer *buffer) { if (buffer->class00!=0x7b711c) abort(); }
}
// Native ABI bridge for the single recovered buffer class. Unknown class
// identities cannot dispatch through the player's original function addresses.
extern "C" unsigned int UG2FxBufferAddRefVirtual(D3DXEffectBuffer *buffer) {
    Check(buffer); return D3DXEffectBufferAddRef(buffer);
}
extern "C" D3DXEffectBuffer *UG2FxBufferDeleteVirtual(D3DXEffectBuffer *buffer,unsigned char flags) {
    Check(buffer); return buffer->Delete(flags);
}
extern "C" int UG2FxBufferAllocateVirtual(D3DXEffectBuffer *buffer,unsigned int size) {
    Check(buffer); return buffer->Allocate(size);
}
