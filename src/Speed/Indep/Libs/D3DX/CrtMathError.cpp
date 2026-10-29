#include "CrtFloor.h"
// Original 0x7661ba: the CRT errno field is thread-data +8.
extern "C" int *__cdecl D3DXCrtErrno() { return reinterpret_cast<int *>(static_cast<char *>(UG2FxCrtThreadData())+8); }
// Original 0x767485. The pointer-valued return for handled errors is observable.
extern "C" int __cdecl D3DXCrtSetMathErrno(int type) {
    if (type==1) { int *value=D3DXCrtErrno();*value=0x21;return reinterpret_cast<int>(value); }
    if (type>1 && type<4) { int *value=D3DXCrtErrno();*value=0x22;return reinterpret_cast<int>(value); }
    return type;
}
// Original 0x7674da. These payload copies are integer MOVs, not FP loads.
extern "C" long double __cdecl D3DXCrtUserMathError(int type,unsigned int operation,D3DXCrtDoubleBits first,D3DXCrtDoubleBits second,D3DXCrtDoubleBits result,unsigned int control) {
    const char *name=0;
    for (int i=0;i<29;++i) if (ug2CrtMathNames81A838[i].operation00==operation) { name=ug2CrtMathNames81A838[i].name04;break; }
    if (name) {
        D3DXCrtMathException error;error.type00=type;error.name04=name;
        UG2_CRT_COPY_DOUBLE_BITS(error.input08,first);UG2_CRT_COPY_DOUBLE_BITS(error.input10,second);UG2_CRT_COPY_DOUBLE_BITS(error.result18,result);
        D3DXCrtRawControl(control,0xffff);
        if (!D3DXCrtDefaultMathError(&error)) D3DXCrtSetMathErrno(type);
        return error.result18;
    }
    D3DXCrtRawControl(control,0xffff);D3DXCrtSetMathErrno(type);return *reinterpret_cast<const double *>(&result);
}
// Original 0x767578. The nonzero mode bypasses user matherr but sets errno.
extern "C" long double __cdecl D3DXCrtHandleQnan(unsigned int operation,double value,unsigned int control) {
    if (!ug2CrtMathErrorFlag81ACE0) {
        double first,result;UG2_FX_VM_STORE_LOAD(result,value);UG2_FX_VM_STORE_LOAD(first,value);
        D3DXCrtDoubleBits firstBits,resultBits,zero={0,0};
        UG2_CRT_COPY_DOUBLE_BITS(firstBits,first);UG2_CRT_COPY_DOUBLE_BITS(resultBits,result);
        return D3DXCrtUserMathError(1,operation,firstBits,zero,resultBits,control);
    }
    *D3DXCrtErrno()=0x21;D3DXCrtRawControl(control,0xffff);return value;
}
// Original complete 0x7694d0..0x769763. Keep signed operation selection,
// alias-sensitive result copies, callback publication and errno fallback.
extern "C" void __cdecl D3DXCrtSseMathError(double *first,double *second,double *output,int operation) {
    int type=0,errnoValue=0;const char *name=0;bool copyFirst=false;
    switch (operation) {
    case 2:type=2;name=reinterpret_cast<const char *>(0x7b3b14);errnoValue=0x22;break;
    case 3:type=1;name=reinterpret_cast<const char *>(0x7b3b14);errnoValue=0x21;break;
    case 8:type=2;name=reinterpret_cast<const char *>(0x7b3b0c);errnoValue=0x22;break;
    case 9:type=1;name=reinterpret_cast<const char *>(0x7b3b0c);errnoValue=0x21;break;
    case 14:type=3;name=reinterpret_cast<const char *>(0x7b3ba0);errnoValue=0x22;break;
    case 15:type=4;name=reinterpret_cast<const char *>(0x7b3ba0);break;
    case 24:type=3;name=reinterpret_cast<const char *>(0x7b3adc);errnoValue=0x22;break;
    case 25:type=4;name=reinterpret_cast<const char *>(0x7b3adc);break;
    case 26:*output=1.0;return;
    case 27:type=2;name=reinterpret_cast<const char *>(0x7b3adc);errnoValue=0x22;break;
    case 28:type=1;name=reinterpret_cast<const char *>(0x7b3adc);errnoValue=0x21;break;
    case 29:type=1;name=reinterpret_cast<const char *>(0x7b3adc);errnoValue=0x21;copyFirst=true;break;
    case 161:type=3;name=reinterpret_cast<const char *>(0x7b3b98);errnoValue=0x22;break;
    case 162:type=4;name=reinterpret_cast<const char *>(0x7b3b98);errnoValue=0x22;break;
    case 166:type=3;name=reinterpret_cast<const char *>(0x7d90c0);errnoValue=0x22;break;
    case 170:type=2;name=reinterpret_cast<const char *>(0x7b3b04);errnoValue=0x22;break;
    case 171:type=1;name=reinterpret_cast<const char *>(0x7b3b04);errnoValue=0x21;break;
    case 1000:type=1;name=reinterpret_cast<const char *>(0x7b3b14);errnoValue=0x21;copyFirst=true;break;
    case 1001:type=1;name=reinterpret_cast<const char *>(0x7b3b0c);errnoValue=0x21;copyFirst=true;break;
    case 1002:type=1;name=reinterpret_cast<const char *>(0x7b3ba0);errnoValue=0x21;copyFirst=true;break;
    case 1003:type=1;name=reinterpret_cast<const char *>(0x7b3c14);errnoValue=0x21;copyFirst=true;break;
    case 1004:type=1;name=reinterpret_cast<const char *>(0x7b3c00);errnoValue=0x21;copyFirst=true;break;
    case 1005:type=1;name=reinterpret_cast<const char *>(0x7b3b7c);errnoValue=0x21;copyFirst=true;break;
    case 1006:type=1;name=reinterpret_cast<const char *>(0x7b3adc);errnoValue=0x21;break;
    case 1007:type=1;name=reinterpret_cast<const char *>(0x7b3afc);errnoValue=0x21;copyFirst=true;break;
    default:return;
    }
    if (copyFirst) UG2_FX_VM_STORE_LOAD(*output,*first);
    D3DXCrtMathException error;error.type00=type;error.name04=name;
    UG2_FX_VM_STORE_LOAD(error.input08,*first);UG2_FX_VM_STORE_LOAD(error.input10,*second);UG2_FX_VM_STORE_LOAD(error.result18,*output);
    if (!UG2FxCrtCallMathError(ug2CrtMathErrorCallback81AA5C,&error) && errnoValue) *D3DXCrtErrno()=errnoValue;
    UG2_FX_VM_STORE_LOAD(*output,error.result18);
}
