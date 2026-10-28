#ifndef UG2_D3DX_EFFECT_EVALUATION_H
#define UG2_D3DX_EFFECT_EVALUATION_H
#include "EffectEvaluationCopy.h"
extern "C" {
// The private original mappings receive their word in EBX. These ordinary
// source interfaces make the input explicit, without forcing registers.
unsigned int __cdecl D3DXEffectAbstractControl(unsigned int); // 0x75da18
unsigned int __cdecl D3DXEffectHardwareControl(unsigned int); // 0x75daaa
unsigned int __cdecl D3DXEffectControl87(unsigned int,unsigned int); // 0x75db38
unsigned int __cdecl D3DXEffectControlFp(unsigned int,unsigned int); // 0x75db6a
// Platform x87 instructions, separated from recovered CRT bit mapping.
unsigned int __cdecl UG2FxReadControlWord();
void __cdecl UG2FxWriteControlWord(unsigned int);
// ABI bridge to recovered VM 0x688e79. Fourteen stack arguments / RET 0x38.
int __stdcall UG2FxExecuteVm(unsigned int *,unsigned int *,unsigned int,unsigned int,unsigned int,
    unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int);
}
#endif
