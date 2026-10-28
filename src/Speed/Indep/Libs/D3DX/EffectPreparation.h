#ifndef UG2_D3DX_EFFECT_PREPARATION_H
#define UG2_D3DX_EFFECT_PREPARATION_H
#include "EffectCompiledPayload.h"
#include "EffectShaderResources.h"
extern "C" {
int __stdcall D3DXEffectVectorOnly(unsigned int **type); // 0x65b098
// Shader interface +0xc (GetDevice) and +0x10 (GetFunction). The dispatch
// argument retains vtable reads made before subsequent buffer getter calls.
int __stdcall UG2FxShaderDevice(unsigned int dispatch,unsigned int shader,unsigned int *device);
int __stdcall UG2FxShaderFunction(unsigned int dispatch,unsigned int shader,void *data,unsigned int *bytes);
}
#endif
