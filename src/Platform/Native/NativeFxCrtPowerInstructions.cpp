#include "../../Speed/Indep/Libs/D3DX/CrtPower.h"
// x87 ISA operations only. Recovered policy and arithmetic order stay in CrtPower.cpp.
extern "C" void __cdecl UG2FxCrtExtendedLoad(D3DXCrtExtended *out,const D3DXCrtExtended *input) {
#ifdef _MSC_VER
    __asm { mov eax,input
        fld tbyte ptr [eax]
        mov eax,out
        fstp tbyte ptr [eax] }
#else
    asm volatile("fldt %1; fstpt %0" : "=m"(*out) : "m"(*input) : "st");
#endif
}
extern "C" void __cdecl UG2FxCrtExtendedRound(D3DXCrtExtended *out,const D3DXCrtExtended *input) {
#ifdef _MSC_VER
    __asm { mov eax,input
        fld tbyte ptr [eax]
        frndint
        mov eax,out
        fstp tbyte ptr [eax] }
#else
    asm volatile("fldt %1; frndint; fstpt %0" : "=m"(*out) : "m"(*input) : "st");
#endif
}
extern "C" void __cdecl UG2FxCrtExtendedAdd(D3DXCrtExtended *out,const D3DXCrtExtended *a,const D3DXCrtExtended *b) {
#ifdef _MSC_VER
    __asm { mov eax,a
        fld tbyte ptr [eax]
        mov eax,b
        fld tbyte ptr [eax]
        faddp st(1),st(0)
        mov eax,out
        fstp tbyte ptr [eax] }
#else
    asm volatile("fldt %1; fldt %2; faddp; fstpt %0" : "=m"(*out) : "m"(*a),"m"(*b) : "st");
#endif
}
extern "C" void __cdecl UG2FxCrtExtendedSubtract(D3DXCrtExtended *out,const D3DXCrtExtended *a,const D3DXCrtExtended *b) {
#ifdef _MSC_VER
    __asm { mov eax,b
        fld tbyte ptr [eax]
        mov eax,a
        fld tbyte ptr [eax]
        fsubrp st(1),st(0)
        mov eax,out
        fstp tbyte ptr [eax] }
#else
    asm volatile("fldt %1; fldt %2; fsubp; fstpt %0" : "=m"(*out) : "m"(*b),"m"(*a) : "st");
#endif
}
extern "C" void __cdecl UG2FxCrtExtendedFyl2x(D3DXCrtExtended *out,const D3DXCrtExtended *a,const D3DXCrtExtended *b) {
#ifdef _MSC_VER
    __asm { mov eax,a
        fld tbyte ptr [eax]
        mov eax,b
        fld tbyte ptr [eax]
        fyl2x
        mov eax,out
        fstp tbyte ptr [eax] }
#else
    asm volatile("fldt %1; fldt %2; fyl2x; fstpt %0" : "=m"(*out) : "m"(*a),"m"(*b) : "st");
#endif
}
extern "C" void __cdecl UG2FxCrtExtendedNegate(D3DXCrtExtended *value) {
#ifdef _MSC_VER
    __asm { mov eax,value
        fld tbyte ptr [eax]
        fchs
        fstp tbyte ptr [eax] }
#else
    asm volatile("fldt %0; fchs; fstpt %0" : "+m"(*value) : : "st");
#endif
}
extern "C" void __cdecl UG2FxCrtExtendedF2xm1(D3DXCrtExtended *value) {
#ifdef _MSC_VER
    __asm { mov eax,value
        fld tbyte ptr [eax]
        f2xm1
        fstp tbyte ptr [eax] }
#else
    asm volatile("fldt %0; f2xm1; fstpt %0" : "+m"(*value) : : "st");
#endif
}
extern "C" void __cdecl UG2FxCrtExtendedAbs(D3DXCrtExtended *out,const D3DXCrtExtended *value) {
#ifdef _MSC_VER
    __asm { mov eax,value
        fld tbyte ptr [eax]
        fabs
        mov eax,out
        fstp tbyte ptr [eax] }
#else
    asm volatile("fldt %1; fabs; fstpt %0" : "=m"(*out) : "m"(*value) : "st");
#endif
}
extern "C" void __cdecl UG2FxCrtExtendedScale(D3DXCrtExtended *value,const D3DXCrtExtended *exponent) {
#ifdef _MSC_VER
    __asm { mov eax,exponent
        fld tbyte ptr [eax]
        mov eax,value
        fld tbyte ptr [eax]
        fscale
        fstp st(1)
        fstp tbyte ptr [eax] }
#else
    asm volatile("fldt %1; fldt %0; fscale; fstp %%st(1); fstpt %0" : "+m"(*value) : "m"(*exponent) : "st");
#endif
}
extern "C" void __cdecl UG2FxCrtExtendedMultiplyDouble(D3DXCrtExtended *value,const double *other) {
#ifdef _MSC_VER
    __asm { mov eax,value
        fld tbyte ptr [eax]
        mov ecx,other
        fmul qword ptr [ecx]
        fstp tbyte ptr [eax] }
#else
    asm volatile("fldt %0; fmull %1; fstpt %0" : "+m"(*value) : "m"(*other) : "st");
#endif
}
extern "C" unsigned int __cdecl UG2FxCrtExtendedCompare(const D3DXCrtExtended *a,const D3DXCrtExtended *b) {
    unsigned short status;
#ifdef _MSC_VER
    __asm { mov eax,b
        fld tbyte ptr [eax]
        mov eax,a
        fld tbyte ptr [eax]
        fcompp
        fstsw status }
#else
    asm volatile("fldt %2; fldt %1; fcompp; fstsw %0" : "=m"(status) : "m"(*a),"m"(*b) : "st");
#endif
    return status;
}
extern "C" unsigned int __cdecl UG2FxCrtExtendedCompareDouble(const D3DXCrtExtended *a,const double *b) {
    unsigned short status;
#ifdef _MSC_VER
    __asm { mov eax,a
        fld tbyte ptr [eax]
        mov eax,b
        fcomp qword ptr [eax]
        fstsw status }
#else
    asm volatile("fldt %1; fcompl %2; fstsw %0" : "=m"(status) : "m"(*a),"m"(*b) : "st");
#endif
    return status;
}
extern "C" void __cdecl UG2FxCrtSaveEnvironment(D3DXCrtSavedEnvironment *out) {
#ifdef _MSC_VER
    __asm { mov eax,out
        fsave [eax] }
#else
    asm volatile("fsave %0" : "=m"(*out) : : "st","memory");
#endif
}
extern "C" void __cdecl UG2FxCrtRestoreEnvironment(const D3DXCrtSavedEnvironment *value) {
#ifdef _MSC_VER
    __asm { mov eax,value
        frstor [eax] }
#else
    asm volatile("frstor %0" : : "m"(*value) : "st","memory");
#endif
}
