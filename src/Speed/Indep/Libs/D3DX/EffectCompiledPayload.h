#ifndef UG2_D3DX_EFFECT_COMPILED_PAYLOAD_H
#define UG2_D3DX_EFFECT_COMPILED_PAYLOAD_H
#include "EffectMetadata.h"
extern "C" {
int __stdcall D3DXEffectFindComment(const unsigned int *tokens,unsigned int tag,unsigned int **data,unsigned int *bytes); // 0x671db5
int __stdcall D3DXEffectReadConstantTable(const unsigned int *tokens,unsigned int **table,unsigned short *counts,unsigned short *minimum,unsigned short *maximum); // 0x65b3f7
int __stdcall D3DXEffectWalkOperands(unsigned int count,unsigned int **cursor); // 0x688d2c
int __stdcall D3DXEffectWalkResult(unsigned int **cursor); // 0x688d8c
int __stdcall D3DXEffectReadCompiledSections(const unsigned int *tokens,unsigned int **instructions,unsigned int **literals); // 0x688dc9
}
#endif
