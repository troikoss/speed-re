#include "CrtPower.h"
static void Copy(D3DXCrtExtended &to,const D3DXCrtExtended &from) { to.low=from.low;to.high=from.high;to.exponent=from.exponent; }
static D3DXCrtDoubleBits Bits(const double &value) { D3DXCrtDoubleBits bits;UG2_CRT_COPY_DOUBLE_BITS(bits,value);return bits; }
static const double &Number(const D3DXCrtDoubleBits &bits) { return *reinterpret_cast<const double *>(&bits); }
// Original 0x75dc8e. Nonfinite classification includes an explicit double copy
// before the already recovered bit classifier. Preserve its NaN quieting flags.
extern "C" int __cdecl D3DXCrtFpClass(D3DXCrtDoubleBits value) {
    unsigned int exponent=value.high&0x7ff00000;bool negative=(value.high&0x80000000)!=0;
    if (exponent==0x7ff00000) {
        double prepared;UG2_FX_VM_STORE_LOAD(prepared,Number(value));int type=D3DXCrtSpecialType(prepared);
        if (type==1) return 0x200;if (type==2) return 4;if (type==3) return 2;return 1;
    }
    if (!exponent&&((value.high&0xfffff)||value.low)) return negative ? 0x10 : 0x80;
    if (UG2_FX_VM_EQ(Number(value),ug2VmZero786E60)) return negative ? 0x20 : 0x40;
    return negative ? 8 : 0x100;
}
// Original 0x76584c. Subnormals are rejected before rounding. Infinities pass
// through both ordered equal tests; NaNs fail them. Keep the separate copies.
extern "C" int __cdecl D3DXCrtPowerParity(D3DXCrtDoubleBits value) {
    double prepared;UG2_FX_VM_STORE_LOAD(prepared,Number(value));
    if (D3DXCrtFpClass(Bits(prepared))&0x90) return 0;
    UG2_FX_VM_STORE_LOAD(prepared,Number(value));double rounded=D3DXCrtRoundCurrent(prepared);
    if (!UG2_FX_VM_EQ(rounded,Number(value))) return 0;
    double half=static_cast<long double>(Number(value))/ug2VmTwo787AD0;
    UG2_FX_VM_STORE_LOAD(prepared,half);rounded=D3DXCrtRoundCurrent(prepared);
    return UG2_FX_VM_EQ(rounded,half) ? 2 : 1;
}
// Original 0x75cbd2. The private helper consumes ST0. Its SAHF/ZF tests include
// unordered comparisons; represent the CL result directly without changing it
// to an ordered IEEE integer predicate.
extern "C" unsigned int __cdecl D3DXCrtPowerParity80(const D3DXCrtExtended *value) {
    D3DXCrtExtended rounded;UG2FxCrtExtendedRound(&rounded,value);
    if (!(UG2FxCrtExtendedCompare(&rounded,value)&0x4000)) return 0;
    D3DXCrtExtended half;Copy(half,*value);UG2FxCrtExtendedMultiplyDouble(&half,&ug2CrtPowerHalf819C80);
    UG2FxCrtExtendedRound(&rounded,&half);return (UG2FxCrtExtendedCompare(&rounded,&half)&0x4000) ? 2 : 1;
}
// Original 0x765705. Nonfinite arguments are built as an extended bit pattern
// before FLD; the ECX scratch change is an observed private-call side effect.
extern "C" unsigned int __cdecl D3DXCrtPowerLoadOperand(D3DXCrtExtended *out,D3DXCrtDoubleBits value,unsigned int *carry) {
    unsigned int exponent=value.high&0x7ff00000;
    if (exponent!=0x7ff00000) { UG2FxCrtExtendedFromDouble(out,&Number(value));return exponent; }
    D3DXCrtExtended prepared;prepared.low=value.low<<11;prepared.high=(value.high<<11)|(value.low>>21);prepared.exponent=static_cast<unsigned short>((value.high>>16)|0x7fff);
    *carry=value.low<<11;
    // FLD's quieting/flags are instruction behavior, including extended NaNs.
    UG2FxCrtExtendedLoad(out,&prepared);
    return value.high;
}
// Original 0x7656c0. Keep rounded-minus-input, then FCHS, F2XM1, +1 and FSCALE.
extern "C" void __cdecl D3DXCrtReconstructExp2(D3DXCrtExtended *value) {
    D3DXCrtExtended rounded,difference,result;
    UG2FxCrtExtendedRound(&rounded,value);UG2FxCrtExtendedSubtract(&difference,&rounded,value);UG2FxCrtExtendedNegate(&difference);
    UG2FxCrtExtendedF2xm1(&difference);UG2FxCrtExtendedAddOne(&result,&difference);UG2FxCrtExtendedScale(&result,&rounded);Copy(*value,result);
}
// Original 0x7658b6. Output is deliberately untouched for unhandled inputs.
extern "C" int __cdecl D3DXCrtPowerSpecial(D3DXCrtDoubleBits base,D3DXCrtDoubleBits exponent,double *out) {
    double magnitude;
    if (UG2_FX_VM_LT(Number(base),ug2VmZero786E60)) UG2_FX_VM_STORE_NEG(magnitude,Number(base));else UG2_FX_VM_STORE_LOAD(magnitude,Number(base));
    int error=0;
    if (exponent.high==0x7ff00000&&!exponent.low) {
        if (UG2_FX_VM_GT(magnitude,ug2VmOne786E68)) UG2_FX_VM_STORE_LOAD(*out,ug2CrtInfinity81A810);
        else if (UG2_FX_VM_LT(magnitude,ug2VmOne786E68)) *out=0.0;
        else { UG2_FX_VM_STORE_LOAD(*out,ug2CrtPowerIndefinite81A818);error=1; }
    } else if (exponent.high==0xfff00000&&!exponent.low) {
        if (UG2_FX_VM_GT(magnitude,ug2VmOne786E68)) *out=0.0;
        else if (UG2_FX_VM_LT(magnitude,ug2VmOne786E68)) UG2_FX_VM_STORE_LOAD(*out,ug2CrtInfinity81A810);
        else { UG2_FX_VM_STORE_LOAD(*out,ug2CrtPowerIndefinite81A818);error=1; }
    } else if (base.high==0x7ff00000&&!base.low) {
        if (UG2_FX_VM_GT(Number(exponent),ug2VmZero786E60)) UG2_FX_VM_STORE_LOAD(*out,ug2CrtInfinity81A810);
        else if (UG2_FX_VM_LT(Number(exponent),ug2VmZero786E60)) *out=0.0;else *out=1.0;
    } else if (base.high==0xfff00000&&!base.low) {
        double prepared;UG2_FX_VM_STORE_LOAD(prepared,Number(exponent));int parity=D3DXCrtPowerParity(Bits(prepared));
        if (UG2_FX_VM_GT(Number(exponent),ug2VmZero786E60)) {
            double result;if (parity==1) UG2_FX_VM_STORE_NEG(result,ug2CrtInfinity81A810);else UG2_FX_VM_STORE_LOAD(result,ug2CrtInfinity81A810);UG2_FX_VM_STORE_LOAD(*out,result);
        } else if (UG2_FX_VM_LT(Number(exponent),ug2VmZero786E60)) {
            double result;if (parity==1) UG2_FX_VM_STORE_LOAD(result,ug2CrtPowerNegativeZero81A830);else result=0.0;UG2_FX_VM_STORE_LOAD(*out,result);
        } else *out=1.0;
    }
    return error;
}
// Original 0x765660 prefix and shared tail 0x765680..0x7656b2. The second
// argument is copied with integer instructions before the floating result spill.
extern "C" void __cdecl D3DXCrtTwoArgError(unsigned int operation,unsigned int type,const char *name,D3DXCrtDoubleBits first,D3DXCrtDoubleBits second,unsigned int saved,D3DXCrtExtended *value) {
    D3DXCrtMathException error;error.type00=type;UG2_CRT_COPY_DOUBLE_BITS(error.input10,second);
    UG2FxCrtExtendedToDouble(&error.result18,value);error.name04=name;UG2_CRT_COPY_DOUBLE_BITS(error.input08,first);
    D3DXCrtOneArgDispatch(operation,&error,reinterpret_cast<unsigned short *>(&saved));UG2FxCrtExtendedFromDouble(value,&error.result18);
    if (static_cast<unsigned short>(saved)!=0x27f) UG2FxWriteControlWord(saved);
}
// Original 0x7657a9. This precision-mask gate differs from 0x76576b. Underflow
// and overflow repair first scale the EXTENDED result, then compare its magnitude.
extern "C" void __cdecl D3DXCrtPowerMathExit(unsigned int operation,const char *name,D3DXCrtDoubleBits first,D3DXCrtDoubleBits second,unsigned int saved,D3DXCrtExtended *value) {
    double copy;UG2FxCrtExtendedToDouble(&copy,value);unsigned int exponent=Bits(copy).high&0x7ff00000,type=0;
    if (!exponent||exponent==0x7ff00000) {
        D3DXCrtExtended scale,magnitude;UG2FxCrtExtendedFromDouble(&scale,!exponent ? &ug2CrtPowerScaleUp7D7C6C : &ug2CrtPowerScaleDown7D7C64);
        UG2FxCrtExtendedScale(value,&scale);UG2FxCrtExtendedAbs(&magnitude,value);
        unsigned int status=UG2FxCrtExtendedCompareDouble(&magnitude,!exponent ? &ug2CrtPowerMinimum7D7C5C : &ug2CrtPowerMaximum7D7C54);
        if (!exponent) { type=4;if (status&0x100) UG2FxCrtExtendedMultiplyDouble(value,&ug2CrtPowerZero7D7C7C); }
        else { type=3;if (!(status&0x4100)) UG2FxCrtExtendedMultiplyDouble(value,&ug2CrtPowerInfinity7D7C74); }
    } else if (static_cast<unsigned short>(saved)!=0x27f) {
        if (!(saved&0x20)&&(D3DXCrtReadStatus()&0x20)) type=8;else UG2FxWriteControlWord(saved);
    }
    if (type) {
        if (operation==0x1d) D3DXCrtTwoArgError(operation,type,name,first,second,saved,value);
        else D3DXCrtOneArgError(operation,type,name,first,saved,value);
    }
}
extern "C" void __cdecl D3DXCrtPowerX8780(D3DXCrtExtended *out,D3DXCrtDoubleBits base,D3DXCrtDoubleBits exponent,const D3DXCrtExtended *incomingExponent) {
    unsigned int carry=exponent.high,saved=UG2FxReadControlWord(),type=1;D3DXCrtExtended x,y,value;Copy(y,*incomingExponent);
    if (saved!=0x27f) D3DXCrtPrepareControl(saved);
    carry&=0x7ff00000;bool exponentSpecial=carry==0x7ff00000;if (exponentSpecial) carry=0;
    unsigned int classification=D3DXCrtPowerLoadOperand(&x,base,&carry);bool baseSpecial=(base.high&0x7ff00000)==0x7ff00000;
    bool error=false,quick=false;
    if (exponentSpecial||baseSpecial) {
        bool exponentNan=exponentSpecial&&((exponent.high&0xfffff)||exponent.low);
        if (exponentSpecial) { if (exponentNan && !(exponent.high&0x80000)) ++carry; }
        else carry=0;
        bool baseNan=baseSpecial&&((base.high&0xfffff)||base.low);
        if ((exponentNan&&(exponent.high&0x80000))||baseNan||carry) {
            // The quiet exponent takes precedence before inspecting the base.
            type=(exponentNan&&(exponent.high&0x80000))|| (baseNan&&(base.high&0x80000)) ? 1 : 7;
            UG2FxCrtExtendedAdd(&value,&y,&x);error=true;
        } else {
            double a,b,result;UG2FxCrtExtendedToDouble(&a,&x);UG2FxCrtExtendedToDouble(&b,&y);
            D3DXCrtSavedEnvironment environment;UG2FxCrtSaveEnvironment(&environment);int failed=D3DXCrtPowerSpecial(Bits(a),Bits(b),&result);UG2FxCrtRestoreEnvironment(&environment);
            UG2FxCrtExtendedFromDouble(&value,&result);if (failed) error=true;else quick=true;
        }
    } else if (!(classification&0x7ff00000)&&!((base.high&0xfffff)||base.low)) {
        if (!(exponent.high&0x7fffffff)&&!exponent.low) { value.low=0;value.high=0x80000000;value.exponent=0x3fff;quick=true; }
        else {
            unsigned int parity=D3DXCrtPowerParity80(&y);bool negative=(base.high&0x80000000)&&(parity&1);
            if (exponent.high&0x80000000) { Copy(value,ug2CrtPowerInfinity81A470);if (negative) UG2FxCrtExtendedNegate(&value);type=2;error=true; }
            else { value.low=0;value.high=0;value.exponent=negative ? 0x8000 : 0;quick=true; }
        }
    } else {
        unsigned int parity=0;
        if (base.high&0x80000000) { parity=D3DXCrtPowerParity80(&y);UG2FxCrtExtendedNegate(&x); }
        if ((base.high&0x80000000)&&!parity) { Copy(value,ug2CrtIndefinite81A4D0);error=true; }
        else { UG2FxCrtExtendedFyl2x(&value,&y,&x);D3DXCrtReconstructExp2(&value);if (parity==1) UG2FxCrtExtendedNegate(&value); }
    }
    if (quick||ug2CrtBypassMathExit8B8424) D3DXCrtQuickExit(saved);
    else if (error) D3DXCrtTwoArgError(0x1d,type,reinterpret_cast<const char *>(0x819c88),base,exponent,saved,&value);
    else D3DXCrtPowerMathExit(0x1d,reinterpret_cast<const char *>(0x819c88),base,exponent,saved,&value);
    Copy(*out,value);
}
// Original SSE wrapper 0x764730; the original 0x764749 body is a separate,
// explicitly unsupported platform boundary until its complete recovery.
extern "C" void __cdecl D3DXCrtPowerSseWrapper80(D3DXCrtExtended *out,const D3DXCrtExtended *base,const D3DXCrtExtended *exponent) {
    double a,b,result;UG2FxCrtExtendedToDouble(&a,base);UG2FxCrtExtendedToDouble(&b,exponent);UG2FxCrtPowerSseBody(&result,Bits(a),Bits(b));UG2FxCrtExtendedFromDouble(out,&result);
}
// Original 0x75c9b0. The x87 fallback discards the extended base through its
// double spill, while retaining the incoming extended exponent across its copy.
extern "C" void __cdecl D3DXCrtPower80(D3DXCrtExtended *out,const D3DXCrtExtended *base,const D3DXCrtExtended *exponent) {
    if (ug2CrtSseEnabled8BA480&&(UG2FxCrtReadMxcsr()&0x1f80)==0x1f80&&(UG2FxCrtReadControlNoWait()&0x7f)==0x7f) { D3DXCrtPowerSseWrapper80(out,base,exponent);return; }
    double a,b;UG2FxCrtExtendedToDouble(&a,base);UG2FxCrtExtendedToDouble(&b,exponent);D3DXCrtPowerX8780(out,Bits(a),Bits(b),exponent);
}
