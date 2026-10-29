#include "../../Speed/Indep/Src/World/CarRenderMaterial.h"
#include "../../Speed/Indep/Libs/D3DX/EffectPassRuntime.h"
#include <stdio.h>
#include <stdlib.h>
// Typed boundaries for the renderer backend. Four effect methods now reach
// recovered pass/texture control; actual GPU classes/state dispatch remain open.
// Weak definitions allow native probes to provide explicit, verified service
// fixtures. Production cannot silently accept device/effect work.
namespace {
__attribute__((noreturn)) void Unsupported(const char *name) {
    fprintf(stderr,"unimplemented native renderer API: %s\n",name);abort();
}
D3DXEffectObject *Effect(unsigned int token) {
    D3DXEffectObject *effect=reinterpret_cast<D3DXEffectObject *>(token);
    if (!effect || effect->class00!=0x7b6fe0) Unsupported("unimplemented effect class");
    return effect;
}
}
extern "C" __attribute__((weak)) int UG2EffectBeginPass(unsigned int effect,unsigned int index) { return D3DXEffectBeginPass(Effect(effect),index); }
extern "C" __attribute__((weak)) int UG2EffectCommit(unsigned int effect) { return D3DXEffectCommit(Effect(effect)); }
extern "C" __attribute__((weak)) int UG2EffectEndPass(unsigned int effect) { return D3DXEffectEndPass(Effect(effect)); }
extern "C" __attribute__((weak)) int UG2EffectSetFloat(unsigned int effect,unsigned int parameter,float value) { Unsupported("UG2EffectSetFloat"); }
extern "C" __attribute__((weak)) int UG2EffectSetInt(unsigned int effect,unsigned int parameter,int value) { Unsupported("UG2EffectSetInt"); }
extern "C" __attribute__((weak)) int UG2EffectSetIntArray(unsigned int effect,unsigned int parameter,const int *values,unsigned int count) { Unsupported("UG2EffectSetIntArray"); }
extern "C" __attribute__((weak)) int UG2EffectSetMatrix(unsigned int effect,unsigned int parameter,const WheelMatrix *matrix) { Unsupported("UG2EffectSetMatrix"); }
extern "C" __attribute__((weak)) int UG2EffectSetMatrixArray(unsigned int effect,unsigned int parameter,const WheelMatrix *matrices,unsigned int count) { Unsupported("UG2EffectSetMatrixArray"); }
extern "C" __attribute__((weak)) int UG2EffectSetTexture(unsigned int effect,unsigned int parameter,unsigned int texture) { return D3DXEffectSetTexture(Effect(effect),parameter,texture); }
extern "C" __attribute__((weak)) int UG2EffectSetVector(unsigned int effect,unsigned int parameter,const float *value) { Unsupported("UG2EffectSetVector"); }
extern "C" __attribute__((weak)) int UG2EffectSetVectorArray(unsigned int effect,unsigned int parameter,const float *values,unsigned int count) { Unsupported("UG2EffectSetVectorArray"); }
extern "C" __attribute__((weak)) int UG2RenderDrawImmediate(unsigned int device,unsigned int kind,unsigned int primitives,const void *vertices,unsigned int stride) { Unsupported("UG2RenderDrawImmediate"); }
extern "C" __attribute__((weak)) int UG2RenderLightEnable(unsigned int device,unsigned int index,unsigned int enabled) { Unsupported("UG2RenderLightEnable"); }
extern "C" __attribute__((weak)) int UG2RenderSetLight(unsigned int device,unsigned int index,const CarRenderDeviceLight *light) { Unsupported("UG2RenderSetLight"); }
extern "C" __attribute__((weak)) int UG2RenderSetMaterial(unsigned int device,const CarRenderDeviceMaterial *material) { Unsupported("UG2RenderSetMaterial"); }
extern "C" __attribute__((weak)) int UG2RenderSetState(unsigned int device,unsigned int state,unsigned int value) { Unsupported("UG2RenderSetState"); }
extern "C" __attribute__((weak)) int UG2RenderSetTexture(unsigned int device,unsigned int stage,unsigned int texture) { Unsupported("UG2RenderSetTexture"); }
extern "C" __attribute__((weak)) int UG2RenderSetTransform(unsigned int device,unsigned int state,const WheelMatrix *matrix) { Unsupported("UG2RenderSetTransform"); }
