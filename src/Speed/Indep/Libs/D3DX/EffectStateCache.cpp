#include "EffectStateCache.h"

// Original 0x65aa5d. Convert each nonzero DWORD to all ones separately; each
// upload contains exactly one boolean and propagates its own signed failure.
extern "C" int __stdcall D3DXEffectSetVertexBooleans(unsigned int device,unsigned int start,const unsigned int *data,unsigned int count) {
    for (unsigned int i=0;i<count;++i) {
        unsigned int value=data[i] ? 0xffffffff : 0;
        int result=UG2FxCacheVertexBool(device,start+i,&value,1);
        if (result<0) return result;
    }
    return 0;
}

// Complete 0x660205 (289 bytes) and 0x6603b4 (293 bytes) retain their own
// upload order; the two routines share only this recovered allocation prefix.
static int CacheZeroBuffer(D3DXEffectObject *effect,unsigned short *sizes) {
    unsigned int bytes=sizes[0]*16;
    if (bytes<=sizes[1]*4u) bytes=sizes[1]*4u;
    if (bytes<=sizes[2]*16u) bytes=sizes[2]*16u;
    if (effect->valueC8<bytes) {
        unsigned int *grown=static_cast<unsigned int *>(UG2FxMalloc(bytes));
        if (!grown) return static_cast<int>(0x8007000e);
        for (unsigned int i=0;i<(bytes>>2);++i) grown[i]=0;
        if (effect->valueCC) { UG2FxFree(reinterpret_cast<void *>(effect->valueCC));effect->valueCC=0; }
        effect->valueCC=reinterpret_cast<unsigned int>(grown);effect->valueC8=bytes;
    }
    return 0;
}
int D3DXEffectObject::ResetVertexConstants(int enabled,unsigned short *sizes) {
    int result=CacheZeroBuffer(this,sizes);if (result<0) return result;
    if (enabled) { result=UG2FxCacheSetVertexShader(value0C,0);if (result<0) return result; }
    if (sizes[0]) { result=UG2FxCacheVertexFloat(value0C,0,reinterpret_cast<void *>(valueCC),sizes[0]);if (result<0) return result; }
    if (sizes[1]) { result=D3DXEffectSetVertexBooleans(value0C,0,reinterpret_cast<unsigned int *>(valueCC),sizes[1]);if (result<0) return result; }
    if (sizes[2]) result=UG2FxCacheVertexInt(value0C,0,reinterpret_cast<void *>(valueCC),sizes[2]);
    return result;
}
int D3DXEffectObject::ResetPixelConstants(int enabled,unsigned short *sizes) {
    int result=CacheZeroBuffer(this,sizes);if (result<0) return result;
    if (enabled) { result=UG2FxCacheSetPixelShader(value0C,0);if (result<0) return result; }
    if (sizes[0]) { result=UG2FxCachePixelFloat(value0C,0,reinterpret_cast<void *>(valueCC),sizes[0]);if (result<0) return result; }
    if (sizes[1]) { result=UG2FxCachePixelBool(value0C,0,reinterpret_cast<void *>(valueCC),sizes[1]);if (result<0) return result; }
    if (sizes[2]) result=UG2FxCachePixelInt(value0C,0,reinterpret_cast<void *>(valueCC),sizes[2]);
    return result;
}

// Original 0x660326 (142 bytes) and 0x6604d9 (131 bytes). The legacy
// decompiler labels the first void, but its EAX returns the last API result.
int D3DXEffectObject::ResetVertexSamplers(int,unsigned short *sizes) {
    int result=0;unsigned int i=0,stage=0x101;
    if (sizes[3]) do {
        result=UG2FxCacheSetTexture(value0C,stage,0);if (result<0) return result;
        for (unsigned int offset=0x1c;offset<0x188;offset+=0x1c) {
            const unsigned int *row=ug2FxSamplerDefaults7B6C54[offset/0x1c];
            unsigned int value=row[1] ? reinterpret_cast<const unsigned int *>(row[1])[1] : 0;
            result=UG2FxCacheSetSampler(value0C,stage,row[0]&0xffffff,value);if (result<0) return result;
        }
        ++i;++stage;
    } while (i<sizes[3]);
    return result;
}
int D3DXEffectObject::ResetPixelSamplers(int,unsigned short *sizes) {
    int result=0;unsigned int i=0;
    if (sizes[3]) do {
        result=UG2FxCacheSetTexture(value0C,i,0);if (result<0) return result;
        for (unsigned int offset=0x1c;offset<0x16c;offset+=0x1c) {
            const unsigned int *row=ug2FxSamplerDefaults7B6C54[offset/0x1c];
            unsigned int value=row[1] ? reinterpret_cast<const unsigned int *>(row[1])[1] : 0;
            result=UG2FxCacheSetSampler(value0C,i,row[0]&0xffffff,value);if (result<0) return result;
        }
        ++i;
    } while (i<sizes[3]);
    return result;
}

#define FX_CAPTURE CaptureVertexConstants
#define FX_SLOT 0x94
#define FX_RESET ResetVertexConstants
// Original complete 0x66356e..0x66369e.
#include "EffectStateCacheCapture.inc"
#undef FX_CAPTURE
#undef FX_SLOT
#undef FX_RESET
#define FX_CAPTURE CaptureVertexSamplers
#define FX_SLOT 0x8c
#define FX_RESET ResetVertexSamplers
// Original complete 0x66369f..0x6637cf.
#include "EffectStateCacheCapture.inc"
#undef FX_CAPTURE
#undef FX_SLOT
#undef FX_RESET
#define FX_CAPTURE CapturePixelConstants
#define FX_SLOT 0x98
#define FX_RESET ResetPixelConstants
// Original complete 0x6637d0..0x663900.
#include "EffectStateCacheCapture.inc"
#undef FX_CAPTURE
#undef FX_SLOT
#undef FX_RESET
#define FX_CAPTURE CapturePixelSamplers
#define FX_SLOT 0x90
#define FX_RESET ResetPixelSamplers
// Original complete 0x663901..0x663a31.
#include "EffectStateCacheCapture.inc"
#undef FX_CAPTURE
#undef FX_SLOT
#undef FX_RESET

#define FX_GATE CacheVertexState
#define FX_READY 0x9c
#define FX_SIZES 0xa4
#define FX_CONSTANT_SLOT 0x94
#define FX_SAMPLER_SLOT 0x8c
#define FX_CONSTANT_CAPTURE CaptureVertexConstants
#define FX_SAMPLER_CAPTURE CaptureVertexSamplers
// Original complete 0x66403c..0x664196.
#include "EffectStateCacheGate.inc"
#undef FX_GATE
#undef FX_READY
#undef FX_SIZES
#undef FX_CONSTANT_SLOT
#undef FX_SAMPLER_SLOT
#undef FX_CONSTANT_CAPTURE
#undef FX_SAMPLER_CAPTURE
#define FX_GATE CachePixelState
#define FX_READY 0xa0
#define FX_SIZES 0xac
#define FX_CONSTANT_SLOT 0x98
#define FX_SAMPLER_SLOT 0x90
#define FX_CONSTANT_CAPTURE CapturePixelConstants
#define FX_SAMPLER_CAPTURE CapturePixelSamplers
// Original complete 0x664197..0x6642f1.
#include "EffectStateCacheGate.inc"
#undef FX_GATE
#undef FX_READY
#undef FX_SIZES
#undef FX_CONSTANT_SLOT
#undef FX_SAMPLER_SLOT
#undef FX_CONSTANT_CAPTURE
#undef FX_SAMPLER_CAPTURE
