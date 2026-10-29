#ifndef UG2_D3DX_EFFECT_PASS_RUNTIME_H
#define UG2_D3DX_EFFECT_PASS_RUNTIME_H
#include "EffectSetup.h"
#include "EffectEvaluation.h"
extern "C" {
int __stdcall D3DXEffectBeginPass(D3DXEffectObject *,unsigned int); // 0x66acd2, RET 8
int __stdcall D3DXEffectCommit(D3DXEffectObject *); // 0x66ac17, RET 4
int __stdcall D3DXEffectEndPass(D3DXEffectObject *); // 0x65adcb, RET 4
int __stdcall D3DXEffectSetTexture(D3DXEffectObject *,unsigned int,unsigned int); // 0x66d7d9, RET 12
// Explicit ABI boundaries for original device-state routines still unrecovered.
int UG2FxPassCache66403C(D3DXEffectObject *,int,void *);
int UG2FxPassCache664197(D3DXEffectObject *,int,void *);
int UG2FxPassFastGroup668DFD(D3DXEffectObject *,D3DXEffectGroup64 *);
int UG2FxPassRecord669286(D3DXEffectObject *,unsigned int,unsigned int,unsigned int);
int UG2FxPassCachedRecord66A25F(D3DXEffectObject *,D3DXEffectGroup64 *,D3DXEffectRecord64 *);
}
#endif
