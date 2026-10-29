#include "CrtExceptions.h"
// Original 0x767689 / 0x767696. Both return the signed full status word.
extern "C" int __cdecl D3DXCrtReadStatus() { return static_cast<short>(UG2FxCrtReadStatusWait()); }
extern "C" int __cdecl D3DXCrtClearStatus() { return static_cast<short>(UG2FxCrtClearStatusNoWait()); }
// Original 0x7676c8. Raise the requested hardware conditions in this order.
extern "C" void __cdecl D3DXCrtSetStatus(unsigned int flags) {
    if (flags&1) UG2FxCrtRaiseInvalid();
    if (flags&8) UG2FxCrtRaiseOverflow();
    if (flags&0x10) UG2FxCrtRaiseUnderflow();
    if (flags&4) UG2FxCrtRaiseZeroDivide();
    if (flags&0x20) UG2FxCrtRaisePrecision();
}
// Original 0x7674ad. Priority is not the hardware mask-bit ordering.
extern "C" int __cdecl D3DXCrtErrorCode(unsigned int flags) {
    if (flags&0x20) return 5;
    if (flags&8) return 1;
    if (flags&4) return 2;
    if (flags&1) return 3;
    return (flags&2)<<1;
}
// Original 0x766e7f. The replacement top word uses the ORIGINAL argument,
// while the other six bytes come from its explicit floating load/store.
extern "C" long double __cdecl D3DXCrtSetExponent(D3DXCrtDoubleBits value,int exponent) {
    double copy;UG2_FX_VM_STORE_LOAD(copy,*reinterpret_cast<const double *>(&value));
    unsigned int top=((static_cast<unsigned int>(exponent)+0x3fe)<<4)|((value.high>>16)&0x800f);
    reinterpret_cast<unsigned int *>(&copy)[1]=(reinterpret_cast<unsigned int *>(&copy)[1]&0xffff)|(top<<16);
    return copy;
}
// Original 0x766f04. In particular, negative zero returns positive zero and
// nonfinite values follow the same exponent replacement as normal values.
extern "C" long double __cdecl D3DXCrtDecompose(D3DXCrtDoubleBits value,int *output) {
    double &number=*reinterpret_cast<double *>(&value);
    if (UG2_FX_VM_EQ(number,ug2VmZero786E60)) { *output=0;return 0.0; }
    int exponent;double prepared,result;
    if (!(value.high&0x7ff00000) && ((value.high&0xfffff)||value.low)) {
        bool negative=UG2_FX_VM_LT(number,ug2VmZero786E60);exponent=-1021;
        while (!(value.high&0x100000)) { value.high=(value.high<<1)|(value.low>>31);value.low<<=1;--exponent; }
        value.high&=~0x100000u;if (negative) value.high|=0x80000000;
        UG2_FX_VM_STORE_LOAD(prepared,number);
    } else {
        exponent=static_cast<int>((value.high>>20)&0x7ff)-0x3fe;
        UG2_FX_VM_STORE_LOAD(prepared,number);
    }
    D3DXCrtDoubleBits bits;UG2_CRT_COPY_DOUBLE_BITS(bits,prepared);
    result=D3DXCrtSetExponent(bits,0);*output=exponent;return result;
}
// Original 0x767263. Handle one primary error, then the precision bit. Do not
// merge the primary branches or round the manually shifted underflow payload.
extern "C" int __cdecl D3DXCrtApplyMasks(unsigned int flags,double *result,unsigned int control) {
    unsigned int remaining=flags&0x1f;
    if ((flags&8)&&(control&1)) { D3DXCrtSetStatus(1);remaining&=~8u; }
    else if ((flags&4)&&(control&4)) { D3DXCrtSetStatus(4);remaining&=~4u; }
    else if ((flags&1)&&(control&8)) {
        D3DXCrtSetStatus(8);bool positive=UG2_FX_VM_GT(*result,ug2VmZero786E60);
        unsigned int rounding=control&0xc00;double replacement;
        if (positive) {
            if (rounding==0x400||rounding==0xc00) UG2_FX_VM_STORE_LOAD(replacement,ug2CrtMaximum81A820);
            else UG2_FX_VM_STORE_LOAD(replacement,ug2CrtInfinity81A810);
        } else {
            if (rounding==0x800||rounding==0xc00) UG2_FX_VM_STORE_NEG(replacement,ug2CrtMaximum81A820);
            else UG2_FX_VM_STORE_NEG(replacement,ug2CrtInfinity81A810);
        }
        UG2_FX_VM_STORE_LOAD(*result,replacement);remaining&=~1u;
    } else if ((flags&2)&&(control&0x10)) {
        bool sticky=(flags&0x10)!=0;
        if (!UG2_FX_VM_EQ(*result,ug2VmZero786E60)) {
            double prepared;UG2_FX_VM_STORE_LOAD(prepared,*result);D3DXCrtDoubleBits bits;UG2_CRT_COPY_DOUBLE_BITS(bits,prepared);
            int exponent;double value=D3DXCrtDecompose(bits,&exponent);exponent-=0x600;
            if (exponent < -0x432) { UG2_FX_VM_STORE_MUL(value,value,ug2VmZero786E60);sticky=true; }
            else {
                bool negative=UG2_FX_VM_LT(value,ug2VmZero786E60);
                D3DXCrtDoubleBits &payload=*reinterpret_cast<D3DXCrtDoubleBits *>(&value);payload.high=(payload.high&0xfffff)|0x100000;
                if (exponent < -0x3fd) {
                    for (int shift=-0x3fd-exponent;shift;--shift) {
                        if (payload.low&1) sticky=true;
                        payload.low=(payload.low>>1)|(payload.high<<31);payload.high>>=1;
                    }
                }
                if (negative) UG2_FX_VM_STORE_NEG(value,value);
            }
            UG2_FX_VM_STORE_LOAD(*result,value);
        } else sticky=true;
        if (sticky) D3DXCrtSetStatus(0x10);remaining&=~2u;
    }
    if ((flags&0x10)&&(control&0x20)) { D3DXCrtSetStatus(0x20);remaining&=~0x10u; }
    return remaining==0;
}
// Original 0x766fbe. Preserve read/write ordering for aliases and allow the OS
// handler to replace the record pointer passed through its one-word argument.
extern "C" void __cdecl D3DXCrtRaiseException(unsigned int *record,unsigned int *control,unsigned int flags,unsigned int operation,double *input,double *result) {
    record[1]=0;record[2]=0;record[3]=0;unsigned int code=flags;
    if (flags&0x10) { record[1]|=1;code=0xc000008f; }
    if (flags&2) { record[1]|=2;code=0xc0000093; }
    if (flags&1) { record[1]|=4;code=0xc0000091; }
    if (flags&4) { record[1]|=8;code=0xc000008e; }
    if (flags&8) { record[1]|=0x10;code=0xc0000090; }
    record[2]^=(~(*control<<4)^record[2])&0x10;
    record[2]^=(~(*control<<1)^record[2])&8;
    record[2]^=(~(*control>>1)^record[2])&4;
    record[2]^=(~(*control>>3)^record[2])&2;
    record[2]^=(~(*control>>5)^record[2])&1;
    unsigned int status=D3DXCrtReadStatus();
    if (status&1) record[3]|=0x10;
    if (status&4) record[3]|=8;
    if (status&8) record[3]|=4;
    if (status&0x10) record[3]|=2;
    if (status&0x20) record[3]|=1;
    unsigned int rounding=*control&0xc00;
    if (!rounding) record[0]&=~3u;
    else if (rounding==0x400) record[0]=(record[0]&~2u)|1;
    else if (rounding==0x800) record[0]=(record[0]&~1u)|2;
    else record[0]|=3;
    unsigned int precision=*control&0x300;
    if (!precision) record[0]=(record[0]&~0x14u)|8;
    else if (precision==0x200) record[0]=(record[0]&~0x18u)|4;
    else if (precision==0x300) record[0]&=~0x1cu;
    record[0]^=((operation<<5)^record[0])&0x1ffe0;
    record[8]|=1;record[8]=(record[8]&~0x1cu)|2;
    UG2_FX_VM_STORE_LOAD(*reinterpret_cast<double *>(record+4),*input);
    record[24]|=1;record[24]=(record[24]&~0x1cu)|2;
    UG2_FX_VM_STORE_LOAD(*reinterpret_cast<double *>(record+20),*result);
    D3DXCrtClearStatus();UG2FxCrtOsRaiseException(code,0,1,reinterpret_cast<unsigned int *>(&record));
    if (record[2]&0x10) *control&=~1u;
    if (record[2]&8) *control&=~4u;
    if (record[2]&4) *control&=~8u;
    if (record[2]&2) *control&=~0x10u;
    if (record[2]&1) *control&=~0x20u;
    switch (record[0]&3) {
    case 0:*control&=~0xc00u;break;
    case 1:*control=(*control&~0x800u)|0x400;break;
    case 2:*control=(*control&~0x400u)|0x800;break;
    case 3:*control|=0xc00;break;
    }
    // These original masks clear ROUNDING bits, not the precision field.
    switch ((record[0]>>2)&7) {
    case 0:*control=(*control&~0xc00u)|0x300;break;
    case 1:*control=(*control&~0xc00u)|0x200;break;
    case 2:*control&=~0xc00u;break;
    }
    UG2_FX_VM_STORE_LOAD(*result,*reinterpret_cast<double *>(record+20));
}
// Original 0x7675cb. Keep uninitialized record padding and the one cleared
// argument-valid bit; only fields written by the original are defined.
extern "C" long double __cdecl D3DXCrtHandleResult(unsigned int flags,unsigned int operation,D3DXCrtDoubleBits first,D3DXCrtDoubleBits result,unsigned int control) {
    double *value=reinterpret_cast<double *>(&result);
    if (!D3DXCrtApplyMasks(flags,value,control)) {
        volatile unsigned int record[32];record[16]&=~1u;
        D3DXCrtRaiseException(const_cast<unsigned int *>(record),&control,flags,operation,reinterpret_cast<double *>(&first),value);
    }
    int error=D3DXCrtErrorCode(flags);
    if (!ug2CrtMathErrorFlag81ACE0 && error) {
        double a,b;UG2_FX_VM_STORE_LOAD(b,*value);UG2_FX_VM_STORE_LOAD(a,*reinterpret_cast<double *>(&first));
        D3DXCrtDoubleBits input,output,zero={0,0};UG2_CRT_COPY_DOUBLE_BITS(input,a);UG2_CRT_COPY_DOUBLE_BITS(output,b);
        return D3DXCrtUserMathError(error,operation,input,zero,output,control);
    }
    D3DXCrtSetMathErrno(error);D3DXCrtRawControl(control,0xffff);return *value;
}
