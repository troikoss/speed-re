#ifndef UG2_D3DX_EFFECT_SORT_H
#define UG2_D3DX_EFFECT_SORT_H
#include "EffectPreparation.h"
typedef int (__cdecl *D3DXEffectCompare)(const void *,const void *);
extern "C" {
int __cdecl D3DXEffectCompareConstants(const void *,const void *); // 0x65b64b
void __cdecl D3DXEffectSort(void *,unsigned int,unsigned int,D3DXEffectCompare); // 0x75cc70, original CRT qsort
// Original 0x75cc00 takes high in EAX, low/width/comparator on the stack.
// This source interface makes that private register argument explicit.
void __cdecl D3DXEffectShortSort(unsigned char *low,unsigned char *high,unsigned int width,D3DXEffectCompare);
extern const unsigned int ug2FxConstantOrder7B5048[4];
}
#endif
