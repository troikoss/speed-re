#include "CrtFloor.h"
#ifdef _MSC_VER
typedef unsigned __int64 CrtFloorBits;
#else
typedef unsigned long long CrtFloorBits;
#endif

// Original raw hardware control helper 0x7676a4. Its return is signed 16-bit,
// unlike the separately recovered CRT abstract-control wrappers.
extern "C" int __cdecl D3DXCrtRawControl(unsigned int value,unsigned int mask) {
    unsigned int previous=UG2FxReadControlWord();
    UG2FxWriteControlWord((previous&~mask)|(value&mask));
    return static_cast<short>(previous);
}
// Original 0x76cc73: FRNDINT uses the current control word, then a double spill.
extern "C" long double __cdecl D3DXCrtRoundCurrent(double value) {
    double result;UG2FxCrtRoundStore(&result,&value);return result;
}
// Original 0x766ea9. Inspect the argument bits without an FP load/quieting step.
extern "C" int __cdecl D3DXCrtSpecialType(double value) {
    const unsigned int *words=reinterpret_cast<const unsigned int *>(&value);
    unsigned int low=words[0],high=words[1];
    if (high==0x7ff00000 && !low) return 1;
    if (high==0xfff00000 && !low) return 2;
    unsigned int tag=(high>>16)&0x7ff8;
    if (tag==0x7ff8) return 3;
    if (tag==0x7ff0 && ((high&0x7ffff) || low)) return 4;
    return 0;
}
// Original 0x75eb50..0x75ec70 plus 0x769764..0x769834. SSE comparisons
// precede some exponent gates; retain their DAZ and exception-flag behavior.
extern "C" long double __cdecl D3DXCrtFloor(double value) {
    if (ug2CrtSseEnabled8BA480 && (UG2FxCrtReadMxcsr()&0x1f80)==0x1f80 && (UG2FxCrtReadControlNoWait()&0x7f)==0x7f) {
        CrtFloorBits bits=*reinterpret_cast<const CrtFloorBits *>(&value);
        unsigned int exponent=static_cast<unsigned int>(bits>>52);
        unsigned int shiftLow=ug2CrtFloorShift7D3CA0[0]-(exponent&ug2CrtFloorExponentMask7D3CD0[0]);
        CrtFloorBits shift=(static_cast<CrtFloorBits>(ug2CrtFloorShift7D3CA0[1])<<32)|shiftLow;
        CrtFloorBits truncated;UG2FxCrtSseTruncate(&truncated,&bits,&shift);
        if (!(exponent&0x800)) {
            if (exponent<0x3ff) return 0.0;
            if (exponent<=0x432) return *reinterpret_cast<const double *>(&truncated);
        } else {
            unsigned int less=UG2FxCrtSseLess(&bits,&truncated);
            if (exponent<0xbff) {
                less=UG2FxCrtSseLess(&bits,&ug2CrtFloorNegativeZero7D3CC0);
                CrtFloorBits result=((less ? ~static_cast<CrtFloorBits>(0) : 0)|*reinterpret_cast<const CrtFloorBits *>(&ug2CrtFloorNegativeZero7D3CC0))&*reinterpret_cast<const CrtFloorBits *>(&ug2CrtFloorNegativeOne7D3CB0);
                return *reinterpret_cast<const double *>(&result);
            }
            if (exponent<=0xc32) {
                CrtFloorBits adjustment=(less ? ~static_cast<CrtFloorBits>(0) : 0)&*reinterpret_cast<const CrtFloorBits *>(&ug2CrtFloorOne7D3C90),result;
                UG2FxCrtSseSubtract(&result,&truncated,&adjustment);return *reinterpret_cast<const double *>(&result);
            }
        }
        if (UG2FxCrtSseUnordered(&value)) D3DXCrtSseMathError(&value,&value,&value,0x3ed);
        return value;
    }
    unsigned int saved=D3DXCrtRawControl(ug2CrtFloorControl81AA60,0xffff);
    double prepared;UG2_FX_VM_STORE_LOAD(prepared,value);
    const unsigned int *words=reinterpret_cast<const unsigned int *>(&value);
    if ((words[1]&0x7ff00000)==0x7ff00000) {
        int type=D3DXCrtSpecialType(prepared);
        if (type>0 && type<=2) { D3DXCrtRawControl(saved,0xffff);return value; }
        if (type==3) {
            UG2_FX_VM_STORE_LOAD(prepared,value);return D3DXCrtHandleQnan(0xb,prepared,saved);
        }
        double result;UG2_FX_VM_STORE_ADD(result,value,ug2VmOne786E68);
        UG2_FX_VM_STORE_LOAD(prepared,value);return UG2FxCrtHandleResult(8,0xb,prepared,result,saved);
    }
    double result=D3DXCrtRoundCurrent(prepared);
    if (UG2_FX_VM_EQ(result,value) || (saved&0x20)) {
        D3DXCrtRawControl(saved,0xffff);return result;
    }
    UG2_FX_VM_STORE_LOAD(prepared,value);return UG2FxCrtHandleResult(0x10,0xb,prepared,result,saved);
}
