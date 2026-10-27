#include "EffectFactory.h"

// Original 0x664ecf. Constructor writes only the indicated parameter fields.
void D3DXEffectParameter::Construct() {
    value08=0xffffffff;
    value24=0; value20=0; value00=0; value04=0;
    value10=3;
    value3C=0; value34=0; value28=0; value46=0; value40=0;
}
// Original 0x65eb78.
void D3DXEffectValuePrefix::Construct() {
    value00=3; value10=0; value14=0; value0C=0; value18=0;
}
// Original 0x66509e. Preserve every untouched byte, including the flag high word.
D3DXEffectObject *D3DXEffectObject::Construct() {
    class00=0x7b6fe0; interface04=0x7b6e00;
    value10=0; value0C=0; value08=0;
    value48.Construct(); valueE0.Construct();
    *reinterpret_cast<unsigned short *>(&value118)&=0xe478;
    value38=0; value9C=0; valueA4=0; value18=0; value20=0; valueB0=0; value28=0;
    valueC4=0; value100=0; value104=0; valueB4=0; valueC0=0; valueB8=0; valueBC=0;
    valueC8=0; valueCC=0; value30=0; value34=0; value2C=0; valueD0=0; valueD4=0;
    valueD8=0; valueDC=0; value114=0;
    value48.value2C=0; value48.value30=0; value48.value18=0; value48.value1C=0; value48.value14=0;
    value108=0; value10C=0; value110=0; value128=0; value124=0; value12C=0;
    value120=0; value11C=0; value3C=0; refs24=1;
    return this;
}
// Original 0x66afc5. Native ABI dispatch reaches the recovered full destructor.
D3DXEffectObject *D3DXEffectObject::Delete(unsigned char flags) {
    UG2FxDestroyEffect(this);
    if (flags&1) UG2FxFree(this);
    return this;
}
// Original 0x65b8c6 / 0x66c361. Reference arithmetic wraps in 32 bits.
extern "C" unsigned int __stdcall D3DXEffectAddRef(D3DXEffectObject *effect) { return ++effect->refs24; }
extern "C" unsigned int __stdcall D3DXEffectRelease(D3DXEffectObject *effect) {
    --effect->refs24;
    unsigned int count=effect->refs24;
    if (!count) { effect->Delete(1); return 0; }
    return count;
}
