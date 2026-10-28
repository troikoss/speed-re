#ifndef UG2_D3DX_EFFECT_METADATA_H
#define UG2_D3DX_EFFECT_METADATA_H
#include "EffectSetup.h"

typedef void (__fastcall *D3DXEffectConstructor)(void *);
extern "C" {
void __stdcall D3DXEffectConstructArray(void *,unsigned int,unsigned int,D3DXEffectConstructor); // 0x401650
void __fastcall D3DXConstructUse(void *); // 0x65b680
void __fastcall D3DXConstructBase(void *); // 0x65b694
void __fastcall D3DXConstructGroup64(void *); // 0x65b6c9
void __fastcall D3DXConstructReference18(void *); // 0x65b78d
void __fastcall D3DXConstructRecord64(void *); // 0x663ec5
void __fastcall D3DXConstructRecord38(void *); // 0x663f4e
void __fastcall D3DXConstructRecordB4(void *); // 0x663f6c
void __fastcall UG2FxConstructParameter(void *); // ABI bridge to 0x664ecf
unsigned int __stdcall D3DXEffectPaddedSize(unsigned int base,unsigned int *typeOffset,unsigned int *dataOffset); // 0x65bf18
unsigned int __stdcall D3DXEffectValueSize(unsigned int base,unsigned int *typeOffset,unsigned int *dataOffset); // 0x65c033
unsigned int __stdcall D3DXEffectUnitCount(unsigned int *type,unsigned int alignment); // 0x65b019
extern const unsigned int ug2FxTypeTable7B582C[][7];
}
#endif
