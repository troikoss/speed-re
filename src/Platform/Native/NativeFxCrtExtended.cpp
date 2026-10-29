#include "../../Speed/Indep/Libs/D3DX/CrtInverse.h"
// Instruction semantics and extended storage only. No libm calls or CRT policy.
extern "C" void __cdecl UG2FxCrtExtendedFromDouble(D3DXCrtExtended *out,const double *input) {
#ifdef _MSC_VER
    __asm { mov eax,input
        fld qword ptr [eax]
        mov eax,out
        fstp tbyte ptr [eax] }
#else
    asm volatile("fldl %1; fstpt %0" : "=m"(*out) : "m"(*input) : "st");
#endif
}
extern "C" void __cdecl UG2FxCrtExtendedToDouble(double *out,const D3DXCrtExtended *input) {
#ifdef _MSC_VER
    __asm { mov eax,input
        fld tbyte ptr [eax]
        mov eax,out
        fstp qword ptr [eax] }
#else
    asm volatile("fldt %1; fstpl %0" : "=m"(*out) : "m"(*input) : "st");
#endif
}
extern "C" void __cdecl UG2FxCrtExtendedAddDouble(D3DXCrtExtended *value,const double *other) {
#ifdef _MSC_VER
    __asm { mov eax,value
        fld tbyte ptr [eax]
        mov ecx,other
        fadd qword ptr [ecx]
        fstp tbyte ptr [eax] }
#else
    asm volatile("fldt %0; faddl %1; fstpt %0" : "+m"(*value) : "m"(*other) : "st");
#endif
}
extern "C" void __cdecl UG2FxCrtExtendedAddOne(D3DXCrtExtended *out,const D3DXCrtExtended *input) {
#ifdef _MSC_VER
    __asm { mov eax,input
        fld tbyte ptr [eax]
        fld1
        faddp st(1),st(0)
        mov eax,out
        fstp tbyte ptr [eax] }
#else
    asm volatile("fldt %1; fld1; faddp; fstpt %0" : "=m"(*out) : "m"(*input) : "st");
#endif
}
extern "C" void __cdecl UG2FxCrtExtendedOneMinus(D3DXCrtExtended *out,const D3DXCrtExtended *input) {
#ifdef _MSC_VER
    __asm { mov eax,input
        fld tbyte ptr [eax]
        fld1
        fsubrp st(1),st(0)
        mov eax,out
        fstp tbyte ptr [eax] }
#else
    asm volatile("fldt %1; fld1; fsubp; fstpt %0" : "=m"(*out) : "m"(*input) : "st");
#endif
}
extern "C" void __cdecl UG2FxCrtExtendedMultiply(D3DXCrtExtended *out,const D3DXCrtExtended *a,const D3DXCrtExtended *b) {
#ifdef _MSC_VER
    __asm { mov eax,a
        fld tbyte ptr [eax]
        mov eax,b
        fld tbyte ptr [eax]
        fmulp st(1),st(0)
        mov eax,out
        fstp tbyte ptr [eax] }
#else
    asm volatile("fldt %1; fldt %2; fmulp; fstpt %0" : "=m"(*out) : "m"(*a),"m"(*b) : "st");
#endif
}
extern "C" void __cdecl UG2FxCrtExtendedSqrt(D3DXCrtExtended *value) {
#ifdef _MSC_VER
    __asm { mov eax,value
        fld tbyte ptr [eax]
        fsqrt
        fstp tbyte ptr [eax] }
#else
    asm volatile("fldt %0; fsqrt; fstpt %0" : "+m"(*value) : : "st");
#endif
}
extern "C" void __cdecl UG2FxCrtExtendedAtan2(D3DXCrtExtended *out,const D3DXCrtExtended *y,const D3DXCrtExtended *x) {
#ifdef _MSC_VER
    __asm { mov eax,y
        fld tbyte ptr [eax]
        mov eax,x
        fld tbyte ptr [eax]
        fpatan
        mov eax,out
        fstp tbyte ptr [eax] }
#else
    asm volatile("fldt %1; fldt %2; fpatan; fstpt %0" : "=m"(*out) : "m"(*y),"m"(*x) : "st");
#endif
}
extern "C" void __cdecl UG2FxCrtExtendedPi(D3DXCrtExtended *value) {
#ifdef _MSC_VER
    __asm { fldpi
        mov eax,value
        fstp tbyte ptr [eax] }
#else
    asm volatile("fldpi; fstpt %0" : "=m"(*value) : : "st");
#endif
}
#ifdef _MSC_VER
// MSVC's long double storage is 64-bit. This ABI-only adapter must return the
// actual 80-bit register value without a compiler-generated double spill.
extern "C" __declspec(naked) long double __cdecl UG2FxCrtReturnExtended(const D3DXCrtExtended *) {
    __asm { mov eax,dword ptr [esp+4]
        fld tbyte ptr [eax]
        ret }
}
#else
extern "C" long double __cdecl UG2FxCrtReturnExtended(const D3DXCrtExtended *value) {
    long double result;asm volatile("fldt %1; fstpt %0" : "=m"(result) : "m"(*value) : "st");return result;
}
#endif
