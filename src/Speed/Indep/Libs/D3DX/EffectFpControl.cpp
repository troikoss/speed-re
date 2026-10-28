#include "EffectEvaluation.h"

// Original 0x75da18..0x75daa9. Only the low hardware word is inspected.
extern "C" unsigned int __cdecl D3DXEffectAbstractControl(unsigned int word) {
    unsigned int value=0;
    if (word&1) value=0x10;
    if (word&4) value|=8;
    if (word&8) value|=4;
    if (word&0x10) value|=2;
    if (word&0x20) value|=1;
    if (word&2) value|=0x80000;
    unsigned int rounding=word&0xc00;
    if (rounding==0x400) value|=0x100;
    else if (rounding==0x800) value|=0x200;
    else if (rounding==0xc00) value|=0x300;
    if (!(word&0x300)) value|=0x20000;
    else if ((word&0x300)==0x200) value|=0x10000;
    if (word&0x1000) value|=0x40000;
    return value;
}
// Original 0x75daaa..0x75db37. Unrecognized precision values collapse to zero.
extern "C" unsigned int __cdecl D3DXEffectHardwareControl(unsigned int value) {
    unsigned int word=(value&0x10)!=0;
    if (value&8) word|=4;
    if (value&4) word|=8;
    if (value&2) word|=0x10;
    if (value&1) word|=0x20;
    if (value&0x80000) word|=2;
    unsigned int rounding=value&0x300;
    if (rounding==0x100) word|=0x400;
    else if (rounding==0x200) word|=0x800;
    else if (rounding==0x300) word|=0xc00;
    if (!(value&0x30000)) word|=0x300;
    else if ((value&0x30000)==0x10000) word|=0x200;
    if (value&0x40000) word|=0x1000;
    return word;
}
// Original 0x75db38..0x75db69. The return retains requested abstract bits,
// including bits that cannot be represented in the hardware control word.
extern "C" unsigned int __cdecl D3DXEffectControl87(unsigned int value,unsigned int mask) {
    unsigned int old=D3DXEffectAbstractControl(UG2FxReadControlWord());
    unsigned int result=(old&~mask)|(value&mask);
    UG2FxWriteControlWord(D3DXEffectHardwareControl(result));
    return result;
}
// Original 0x75db6a..0x75db7f. controlfp protects the denormal exception mask.
extern "C" unsigned int __cdecl D3DXEffectControlFp(unsigned int value,unsigned int mask) {
    return D3DXEffectControl87(value,mask&0xfff7ffff);
}
