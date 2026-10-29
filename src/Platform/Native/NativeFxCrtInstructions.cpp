#include "../../Speed/Indep/Libs/D3DX/CrtFloor.h"
// Instruction semantics only. Selection, bit arithmetic and CRT policy stay
// in recovered source. SSE2 adapters are reached only through its CPU gate.
#ifdef _MSC_VER
#define UG2_CRT_SSE
#else
#define UG2_CRT_SSE __attribute__((target("sse2")))
#endif
extern "C" UG2_CRT_SSE unsigned int __cdecl UG2FxCrtReadMxcsr() {
    unsigned int value;
#ifdef _MSC_VER
    __asm { stmxcsr value }
#else
    asm volatile("stmxcsr %0" : "=m"(value));
#endif
    return value;
}
extern "C" unsigned int __cdecl UG2FxCrtReadControlNoWait() {
    unsigned short value;
#ifdef _MSC_VER
    __asm { fnstcw value }
#else
    asm volatile("fnstcw %0" : "=m"(value));
#endif
    return value;
}
extern "C" UG2_CRT_SSE unsigned int __cdecl UG2FxCrtSseLess(const void *a,const void *b) {
    unsigned int result;
#ifdef _MSC_VER
    __asm { mov eax,a
        movq xmm0,qword ptr [eax]
        mov eax,b
        movq xmm1,qword ptr [eax]
        cmpltpd xmm0,xmm1
        movd eax,xmm0
        mov result,eax }
#else
    asm volatile("movq %1,%%xmm0; movq %2,%%xmm1; cmpltpd %%xmm1,%%xmm0; movd %%xmm0,%0" : "=r"(result) : "m"(*static_cast<const double *>(a)),"m"(*static_cast<const double *>(b)) : "xmm0","xmm1");
#endif
    return result;
}
extern "C" UG2_CRT_SSE unsigned int __cdecl UG2FxCrtSseUnordered(const void *a) {
    unsigned char result;
#ifdef _MSC_VER
    __asm { mov eax,a
        movq xmm0,qword ptr [eax]
        ucomisd xmm0,xmm0
        setp result }
#else
    asm volatile("movq %1,%%xmm0; ucomisd %%xmm0,%%xmm0; setp %0" : "=qm"(result) : "m"(*static_cast<const double *>(a)) : "xmm0","cc");
#endif
    return result;
}
extern "C" UG2_CRT_SSE void __cdecl UG2FxCrtSseSubtract(void *out,const void *a,const void *b) {
#ifdef _MSC_VER
    __asm { mov eax,a
        movq xmm0,qword ptr [eax]
        mov eax,b
        movq xmm1,qword ptr [eax]
        subsd xmm0,xmm1
        mov eax,out
        movq qword ptr [eax],xmm0 }
#else
    asm volatile("movq %1,%%xmm0; movq %2,%%xmm1; subsd %%xmm1,%%xmm0; movq %%xmm0,%0" : "=m"(*static_cast<double *>(out)) : "m"(*static_cast<const double *>(a)),"m"(*static_cast<const double *>(b)) : "xmm0","xmm1");
#endif
}
extern "C" UG2_CRT_SSE void __cdecl UG2FxCrtSseTruncate(void *out,const void *a,const void *shift) {
#ifdef _MSC_VER
    __asm { mov eax,a
        movq xmm0,qword ptr [eax]
        mov eax,shift
        movq xmm1,qword ptr [eax]
        psrlq xmm0,xmm1
        psllq xmm0,xmm1
        mov eax,out
        movq qword ptr [eax],xmm0 }
#else
    asm volatile("movq %1,%%xmm0; movq %2,%%xmm1; psrlq %%xmm1,%%xmm0; psllq %%xmm1,%%xmm0; movq %%xmm0,%0" : "=m"(*static_cast<double *>(out)) : "m"(*static_cast<const double *>(a)),"m"(*static_cast<const double *>(shift)) : "xmm0","xmm1");
#endif
}
extern "C" void __cdecl UG2FxCrtRoundStore(double *out,const double *value) {
#ifdef _MSC_VER
    __asm { mov eax,value
        fld qword ptr [eax]
        frndint
        mov eax,out
        fstp qword ptr [eax] }
#else
    asm volatile("fldl %1; frndint; fstpl %0" : "=m"(*out) : "m"(*value) : "st");
#endif
}
