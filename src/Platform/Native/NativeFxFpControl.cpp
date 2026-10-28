#include "../../Speed/Indep/Libs/D3DX/EffectEvaluation.h"
// Native x86 scaffolding for the original waiting FSTCW and FLDCW operations.
// Bit translation and CRT masks remain in the recovered source.
extern "C" unsigned int __cdecl UG2FxReadControlWord() {
    unsigned short controlWord;
#ifdef _MSC_VER
    __asm { fstcw controlWord }
#else
    asm volatile("fstcw %0" : "=m"(controlWord));
#endif
    return controlWord;
}
extern "C" void __cdecl UG2FxWriteControlWord(unsigned int value) {
    unsigned short controlWord=static_cast<unsigned short>(value);
#ifdef _MSC_VER
    __asm { fldcw controlWord }
#else
    asm volatile("fldcw %0" : : "m"(controlWord));
#endif
}
