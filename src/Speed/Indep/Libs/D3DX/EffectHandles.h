#ifndef UG2_D3DX_EFFECT_HANDLES_H
#define UG2_D3DX_EFFECT_HANDLES_H
#include "EffectCompiledPayload.h"
extern "C" {
int __cdecl D3DXEffectAtolBody(const char *); // 0x75d5d3
int __cdecl D3DXEffectAtol(const char *); // 0x75d65b
extern unsigned int ug2FxCrtLocale819D24;
const void * __cdecl UG2FxCrtGetThreadData(); // 0x761e3c boundary
const void * __cdecl UG2FxCrtUpdateLocale(); // 0x75e3d3 boundary
unsigned int __cdecl UG2FxCrtClassify(const void *locale,unsigned int character,unsigned int mask); // 0x766361 boundary
}
#endif
