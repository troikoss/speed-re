#ifndef UG2_D3DX_EFFECT_SHARED_POOL_H
#define UG2_D3DX_EFFECT_SHARED_POOL_H
#include "EffectValues.h"
extern "C" {
int __cdecl D3DXEffectAsciiCompare(const char *,const char *); // 0x767730
int __cdecl D3DXEffectCaseCompare(const char *,const char *); // 0x75dd6f
unsigned int __cdecl UG2FxCrtLowerLocale(const void *locale,unsigned int character); // 0x75de10, unsupported native locale boundary
int __stdcall D3DXEffectCompareSharedType(unsigned int base,unsigned int **type,unsigned int otherBase,unsigned int **otherType,unsigned int *resources); // 0x6418a2
int __stdcall D3DXEffectMatchSharedType(unsigned int base,unsigned int *type,unsigned int otherBase,unsigned int *otherType,unsigned int *resources); // 0x641a3d
int __stdcall D3DXEffectSharedSize(unsigned int base,unsigned int **type,unsigned int *types,unsigned int *data,unsigned int *names); // 0x641a60
int __stdcall D3DXEffectCopySharedType(unsigned int base,unsigned int **type,unsigned int **input,unsigned int outputBase,unsigned int *names,unsigned int **outputType,unsigned int **outputData,unsigned int *resources); // 0x641c16
int __stdcall D3DXEffectCreateSharedType(unsigned int base,unsigned int *type,unsigned int *input,unsigned int outputBase,unsigned int names,unsigned int namesSize,unsigned int typeSize,unsigned int *resources); // 0x641ec8
int __stdcall D3DXEffectWalkSharedIndices(unsigned int **type,unsigned int **input,unsigned int **output); // 0x641f0d
int __stdcall D3DXEffectSharedIndices(unsigned int *type,unsigned int *input,unsigned int *output); // 0x641ffd
int __stdcall D3DXEffectCopySharedNumeric(unsigned int **type,unsigned int **output,unsigned int **input); // 0x642017
int __stdcall D3DXEffectRefreshSharedNumeric(unsigned int *type,unsigned int *output,unsigned int *input); // 0x642127
int __stdcall D3DXEffectCopySharedResources(unsigned int **type,unsigned int **data,D3DXEffectPoolLink *source,D3DXEffectPoolLink *destination); // 0x642141
int __stdcall D3DXEffectRefreshSharedResources(unsigned int *type,unsigned int *data,D3DXEffectPoolLink *destination,D3DXEffectPoolLink *source); // 0x642487
}
#endif
