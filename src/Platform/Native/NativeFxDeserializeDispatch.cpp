#include "../../Speed/Indep/Libs/D3DX/EffectMetadata.h"
extern "C" int UG2FxDeserializeEffect(D3DXEffectObject *effect,unsigned int device,D3DXEffectPool *pool,D3DXEffectBuffer *buffer,unsigned int *position) {
    return effect->Deserialize(device,pool,buffer,position);
}
// Adapt the original ECX constructor callback to native C++ member dispatch.
extern "C" void __fastcall UG2FxConstructParameter(void *parameter) { static_cast<D3DXEffectParameter *>(parameter)->Construct(); }
