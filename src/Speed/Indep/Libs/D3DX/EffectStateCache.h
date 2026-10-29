#ifndef UG2_D3DX_EFFECT_STATE_CACHE_H
#define UG2_D3DX_EFFECT_STATE_CACHE_H
#include "EffectDebug.h"
#include "EffectFactory.h"
extern "C" {
int __stdcall D3DXEffectSetVertexBooleans(unsigned int device,unsigned int start,const unsigned int *data,unsigned int count); // 0x65aa5d
extern const unsigned int ug2FxSamplerDefaults7B6C54[][7];
// Original device/state-block virtual calls. Native GPU objects remain a
// platform boundary; these services never manufacture successful captures.
int UG2FxStateBlockBegin(unsigned int device); // device+0xf0
int UG2FxStateBlockEnd(unsigned int device,unsigned int *output); // device+0xf4
int UG2FxStateBlockCapture(unsigned int block); // block+0x10
int UG2FxStateBlockApply(unsigned int block); // block+0x14
unsigned int UG2FxStateBlockRelease(unsigned int block); // block+8
int UG2FxCacheSetVertexShader(unsigned int device,unsigned int shader); // device+0x170
int UG2FxCacheSetPixelShader(unsigned int device,unsigned int shader); // device+0x1ac
int UG2FxCacheVertexFloat(unsigned int device,unsigned int start,const void *data,unsigned int count); // +0x178
int UG2FxCacheVertexInt(unsigned int device,unsigned int start,const void *data,unsigned int count); // +0x180
int UG2FxCacheVertexBool(unsigned int device,unsigned int start,const void *data,unsigned int count); // +0x188
int UG2FxCachePixelFloat(unsigned int device,unsigned int start,const void *data,unsigned int count); // +0x1b4
int UG2FxCachePixelInt(unsigned int device,unsigned int start,const void *data,unsigned int count); // +0x1bc
int UG2FxCachePixelBool(unsigned int device,unsigned int start,const void *data,unsigned int count); // +0x1c4
int UG2FxCacheSetTexture(unsigned int device,unsigned int stage,unsigned int texture); // +0x104
int UG2FxCacheSetSampler(unsigned int device,unsigned int stage,unsigned int state,unsigned int value); // +0x114
}
#endif
