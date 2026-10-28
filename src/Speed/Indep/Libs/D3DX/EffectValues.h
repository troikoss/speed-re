#ifndef UG2_D3DX_EFFECT_VALUES_H
#define UG2_D3DX_EFFECT_VALUES_H
#include "EffectMetadata.h"
extern "C" {
void __fastcall D3DXConstructReference08(void *); // 0x65ee33
int __stdcall D3DXEffectWalkValueReferences(unsigned int **type,unsigned int **data,
    D3DXEffectReference08 **references,unsigned int *count,int indexed); // 0x65eb8f
int __stdcall D3DXEffectValueReferences(unsigned int *type,unsigned int *data,
    D3DXEffectReference08 *references,unsigned int *count,int indexed); // 0x65ee13
int __stdcall D3DXEffectSetValue(D3DXEffectObject *,unsigned int handle,const void *data,unsigned int bytes); // 0x66c8f5
int UG2FxSetValueVirtual(D3DXEffectObject *,unsigned int handle,const void *data,unsigned int bytes); // vtable+0x50
}
#endif
