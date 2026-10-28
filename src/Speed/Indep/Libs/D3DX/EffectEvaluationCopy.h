#ifndef UG2_D3DX_EFFECT_EVALUATION_COPY_H
#define UG2_D3DX_EFFECT_EVALUATION_COPY_H
#include "EffectMetadata.h"
extern "C" {
// Seven stack arguments / RET 0x1c. ECX is unused in these four original bodies.
void __stdcall D3DXEffectCopyEvaluationVectors(D3DXEffectBaseRecord *,double *,unsigned int,unsigned int,unsigned int,unsigned int *,unsigned int); // 0x662ecc
void __stdcall D3DXEffectCopyEvaluationScalars(D3DXEffectBaseRecord *,double *,unsigned int,unsigned int,unsigned int,unsigned int *,unsigned int); // 0x663265
void __stdcall D3DXEffectWalkEvaluationVectors(unsigned int **,unsigned int **,double **,unsigned int,unsigned int,unsigned int *,unsigned int); // 0x65fab5
void __stdcall D3DXEffectWalkEvaluationScalars(unsigned int **,unsigned int **,double **,unsigned int,unsigned int,unsigned int *,unsigned int); // 0x65febf
}
#endif
