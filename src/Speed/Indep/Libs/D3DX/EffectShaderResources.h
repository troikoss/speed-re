#ifndef UG2_D3DX_EFFECT_SHADER_RESOURCES_H
#define UG2_D3DX_EFFECT_SHADER_RESOURCES_H
#include "EffectMetadata.h"
extern "C" {
// Unrecovered library/debug and GPU calls retain their own boundaries. The
// cached class token is loaded before the buffer getter, while the device
// argument is reread afterward, as in the original indirect call sequence.
void __stdcall UG2FxDebugMute(unsigned int mute); // 0x671aed
int __stdcall UG2FxCreateVertexShader(unsigned int dispatch,unsigned int device,const void *code,unsigned int *output); // vtable +0x16c
int __stdcall UG2FxCreatePixelShader(unsigned int dispatch,unsigned int device,const void *code,unsigned int *output); // vtable +0x1a8
}
#endif
