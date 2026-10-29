#include "CrtInverse.h"
// Original 0x765748. The final CMP's ZF is represented explicitly by callers;
// for nonfinite input the full high word is returned without changing that ZF.
extern "C" unsigned int __cdecl D3DXCrtInspectOperand(D3DXCrtDoubleBits value) {
    unsigned int exponent=value.high&0x7ff00000;return exponent==0x7ff00000 ? value.high : exponent;
}
// Original 0x7656d5. Keep precision, mask exceptions and select nearest rounding.
extern "C" void __cdecl D3DXCrtPrepareControl(unsigned int saved) { UG2FxWriteControlWord((saved&0x300)|0x7f); }
// Original 0x7656ec. Quiet NaNs bypass the floating operation completely.
extern "C" unsigned int __cdecl D3DXCrtPrepareNan(D3DXCrtExtended *value,unsigned int high) {
    if (high&0x80000) return 7;
    UG2FxCrtExtendedAddDouble(value,&ug2CrtNanAdd7D7C40);return 1;
}
// Original 0x76cb75. The supplied control pointer is read, never updated.
// Error types 7/8 have special callback/precision treatment distinct from UMATHERR.
extern "C" void __cdecl D3DXCrtOneArgDispatch(unsigned int operation,D3DXCrtMathException *error,unsigned short *saved) {
    unsigned int control=*saved,flags=0;
    switch (error->type00) {
    case 1:case 5:flags=8;break;
    case 2:flags=4;break;
    case 3:flags=0x11;break;
    case 4:flags=0x12;break;
    case 7:error->type00=1;break;
    case 8:flags=0x10;break;
    }
    if (flags && !D3DXCrtApplyMasks(flags,&error->result18,control)) {
        volatile unsigned int record[32];
        if (operation==0x10||operation==0x16||operation==0x1d) {
            UG2_FX_VM_STORE_LOAD(*reinterpret_cast<double *>(const_cast<unsigned int *>(record)+12),error->input10);
            record[16]=(record[16]&~0x1cu)|3;
        } else record[16]&=~1u;
        D3DXCrtRaiseException(const_cast<unsigned int *>(record),&control,flags,operation,&error->input08,&error->result18);
    }
    D3DXCrtRawControl(control,0xffff);
    if (error->type00!=8 && !ug2CrtMathErrorFlag81ACE0 && D3DXCrtDefaultMathError(error)) return;
    D3DXCrtSetMathErrno(error->type00);
}
// Original 0x765677. The second input is deliberately uninitialized. The result
// is reloaded BEFORE restoring the saved original word a second time.
extern "C" void __cdecl D3DXCrtOneArgError(unsigned int operation,unsigned int type,const char *name,D3DXCrtDoubleBits input,unsigned int saved,D3DXCrtExtended *value) {
    D3DXCrtMathException error;error.type00=type;
    UG2FxCrtExtendedToDouble(&error.result18,value);error.name04=name;UG2_CRT_COPY_DOUBLE_BITS(error.input08,input);
    D3DXCrtOneArgDispatch(operation,&error,reinterpret_cast<unsigned short *>(&saved));
    UG2FxCrtExtendedFromDouble(value,&error.result18);
    if (static_cast<unsigned short>(saved)!=0x27f) UG2FxWriteControlWord(saved);
}
// Original 0x76575e.
extern "C" void __cdecl D3DXCrtQuickExit(unsigned int saved) { if (static_cast<unsigned short>(saved)!=0x27f) UG2FxWriteControlWord(saved); }
// Original 0x76576b. In particular, the saved precision MASK bit gates this
// error dispatch. Do not invert it into a conventional unmasked-exception test.
extern "C" void __cdecl D3DXCrtMathExit(unsigned int operation,const char *name,D3DXCrtDoubleBits input,unsigned int saved,D3DXCrtExtended *value) {
    if (static_cast<unsigned short>(saved)!=0x27f) {
        if ((saved&0x20)&&(D3DXCrtReadStatus()&0x20)) { D3DXCrtOneArgError(operation,8,name,input,saved,value);return; }
        UG2FxWriteControlWord(saved);
    }
}
static void CopyExtended(D3DXCrtExtended &to,const D3DXCrtExtended &from) { to.low=from.low;to.high=from.high;to.exponent=from.exponent; }
// Complete cores 0x75eebd / 0x75db9d. EAX and ZF from the original classifier
// are explicit arguments. Arithmetic uses original x87 operations; 80-bit storage
// carries values across C++ calls without narrowing. Each wrapper-to-core call
// remains a separate source boundary.
static void InverseCore(D3DXCrtExtended *out,const D3DXCrtExtended *input,D3DXCrtDoubleBits bits,unsigned int classification,bool special,bool cosine) {
    D3DXCrtExtended value;CopyExtended(value,*input);
    unsigned int saved=UG2FxReadControlWord(),type=1;
    unsigned int operation=cosine ? 0xd : 0xe;const char *name=reinterpret_cast<const char *>(cosine ? 0x819cc0 : 0x819f00);
    bool error=false;
    if (special) {
        if ((classification&0xfffff)||bits.low) type=D3DXCrtPrepareNan(&value,classification);
        else CopyExtended(value,ug2CrtIndefinite81A4D0);
        error=true;
    } else {
        if (saved!=0x27f) D3DXCrtPrepareControl(saved);
        if (classification<0x3ff00000) {
            D3DXCrtExtended sum,difference,root,result;
            UG2FxCrtExtendedAddOne(&sum,&value);UG2FxCrtExtendedOneMinus(&difference,&value);
            UG2FxCrtExtendedMultiply(&root,&difference,&sum);UG2FxCrtExtendedSqrt(&root);
            if (cosine) UG2FxCrtExtendedAtan2(&result,&root,&value);else UG2FxCrtExtendedAtan2(&result,&value,&root);
            CopyExtended(value,result);
        } else if (classification>0x3ff00000 || (bits.high&0xfffff)||bits.low) {
            CopyExtended(value,ug2CrtIndefinite81A4D0);error=true;
        } else if (cosine) {
            if (bits.high&0x80000000) UG2FxCrtExtendedPi(&value);
            else { value.low=0;value.high=0;value.exponent=0; }
        } else {
            CopyExtended(value,ug2CrtHalfPi81A4DA);if (bits.high&0x80000000) value.exponent^=0x8000;
        }
    }
    if (ug2CrtBypassMathExit8B8424) D3DXCrtQuickExit(saved);
    else if (error) D3DXCrtOneArgError(operation,type,name,bits,saved,&value);
    else D3DXCrtMathExit(operation,name,bits,saved,&value);
    CopyExtended(*out,value);
}
extern "C" void __cdecl D3DXCrtAsinCore80(D3DXCrtExtended *out,const D3DXCrtExtended *input,D3DXCrtDoubleBits bits,unsigned int classification,unsigned int special) { InverseCore(out,input,bits,classification,special!=0,false); }
extern "C" void __cdecl D3DXCrtAcosCore80(D3DXCrtExtended *out,const D3DXCrtExtended *input,D3DXCrtDoubleBits bits,unsigned int classification,unsigned int special) { InverseCore(out,input,bits,classification,special!=0,true); }
// Original wrappers 0x75eea0 / 0x75db80: retain ST0 across the argument-copy store,
// call the classifier, then call the corresponding core with its EAX/ZF state.
static void InverseEntry(D3DXCrtExtended *out,const D3DXCrtExtended *input,bool cosine) {
    D3DXCrtExtended value;CopyExtended(value,*input);double argument;
    UG2FxCrtExtendedToDouble(&argument,&value);D3DXCrtDoubleBits bits;UG2_CRT_COPY_DOUBLE_BITS(bits,argument);
    unsigned int classification=D3DXCrtInspectOperand(bits);bool special=(bits.high&0x7ff00000)==0x7ff00000;
    if (cosine) D3DXCrtAcosCore80(out,&value,bits,classification,special);else D3DXCrtAsinCore80(out,&value,bits,classification,special);
}
extern "C" void __cdecl D3DXCrtAsin80(D3DXCrtExtended *out,const D3DXCrtExtended *input) { InverseEntry(out,input,false); }
extern "C" void __cdecl D3DXCrtAcos80(D3DXCrtExtended *out,const D3DXCrtExtended *input) { InverseEntry(out,input,true); }
