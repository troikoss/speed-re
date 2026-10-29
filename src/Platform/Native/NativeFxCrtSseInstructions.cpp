#include "../../Speed/Indep/Libs/D3DX/CrtPowerSse.h"
// SSE2 ISA operations only. Reduction, table selection, polynomial order and
// exceptional control flow stay in recovered CrtPowerSse.cpp.
#ifdef _MSC_VER
#define UG2_SSE2
#else
#define UG2_SSE2 __attribute__((target("sse2")))
#endif
extern "C" UG2_SSE2 void __cdecl UG2FxCrtSseAddScalar(D3DXCrtSseValue *value,const D3DXCrtSseValue *other) {
#ifdef _MSC_VER
    __asm { mov eax,value
        movdqu xmm0,xmmword ptr [eax]
        mov ecx,other
        movdqu xmm1,xmmword ptr [ecx]
        addsd xmm0,xmm1
        movdqu xmmword ptr [eax],xmm0 }
#else
    asm volatile("movdqu %0,%%xmm0; movdqu %1,%%xmm1; addsd %%xmm1,%%xmm0; movdqu %%xmm0,%0" : "+m"(*value) : "m"(*other) : "xmm0","xmm1");
#endif
}
extern "C" UG2_SSE2 void __cdecl UG2FxCrtSseSubtractScalar(D3DXCrtSseValue *value,const D3DXCrtSseValue *other) {
#ifdef _MSC_VER
    __asm { mov eax,value
        movdqu xmm0,xmmword ptr [eax]
        mov ecx,other
        movdqu xmm1,xmmword ptr [ecx]
        subsd xmm0,xmm1
        movdqu xmmword ptr [eax],xmm0 }
#else
    asm volatile("movdqu %0,%%xmm0; movdqu %1,%%xmm1; subsd %%xmm1,%%xmm0; movdqu %%xmm0,%0" : "+m"(*value) : "m"(*other) : "xmm0","xmm1");
#endif
}
extern "C" UG2_SSE2 void __cdecl UG2FxCrtSseMultiplyScalar(D3DXCrtSseValue *value,const D3DXCrtSseValue *other) {
#ifdef _MSC_VER
    __asm { mov eax,value
        movdqu xmm0,xmmword ptr [eax]
        mov ecx,other
        movdqu xmm1,xmmword ptr [ecx]
        mulsd xmm0,xmm1
        movdqu xmmword ptr [eax],xmm0 }
#else
    asm volatile("movdqu %0,%%xmm0; movdqu %1,%%xmm1; mulsd %%xmm1,%%xmm0; movdqu %%xmm0,%0" : "+m"(*value) : "m"(*other) : "xmm0","xmm1");
#endif
}
extern "C" UG2_SSE2 void __cdecl UG2FxCrtSseDivideScalar(D3DXCrtSseValue *value,const D3DXCrtSseValue *other) {
#ifdef _MSC_VER
    __asm { mov eax,value
        movdqu xmm0,xmmword ptr [eax]
        mov ecx,other
        movdqu xmm1,xmmword ptr [ecx]
        divsd xmm0,xmm1
        movdqu xmmword ptr [eax],xmm0 }
#else
    asm volatile("movdqu %0,%%xmm0; movdqu %1,%%xmm1; divsd %%xmm1,%%xmm0; movdqu %%xmm0,%0" : "+m"(*value) : "m"(*other) : "xmm0","xmm1");
#endif
}
extern "C" UG2_SSE2 void __cdecl UG2FxCrtSseAddPacked(D3DXCrtSseValue *value,const D3DXCrtSseValue *other) {
#ifdef _MSC_VER
    __asm { mov eax,value
        movdqu xmm0,xmmword ptr [eax]
        mov ecx,other
        movdqu xmm1,xmmword ptr [ecx]
        addpd xmm0,xmm1
        movdqu xmmword ptr [eax],xmm0 }
#else
    asm volatile("movdqu %0,%%xmm0; movdqu %1,%%xmm1; addpd %%xmm1,%%xmm0; movdqu %%xmm0,%0" : "+m"(*value) : "m"(*other) : "xmm0","xmm1");
#endif
}
extern "C" UG2_SSE2 void __cdecl UG2FxCrtSseMultiplyPacked(D3DXCrtSseValue *value,const D3DXCrtSseValue *other) {
#ifdef _MSC_VER
    __asm { mov eax,value
        movdqu xmm0,xmmword ptr [eax]
        mov ecx,other
        movdqu xmm1,xmmword ptr [ecx]
        mulpd xmm0,xmm1
        movdqu xmmword ptr [eax],xmm0 }
#else
    asm volatile("movdqu %0,%%xmm0; movdqu %1,%%xmm1; mulpd %%xmm1,%%xmm0; movdqu %%xmm0,%0" : "+m"(*value) : "m"(*other) : "xmm0","xmm1");
#endif
}
extern "C" UG2_SSE2 void __cdecl UG2FxCrtSseConvertDwords(D3DXCrtSseValue *value) {
#ifdef _MSC_VER
    __asm { mov eax,value
        movdqu xmm0,xmmword ptr [eax]
        cvtdq2pd xmm0,xmm0
        movdqu xmmword ptr [eax],xmm0 }
#else
    asm volatile("movdqu %0,%%xmm0; cvtdq2pd %%xmm0,%%xmm0; movdqu %%xmm0,%0" : "+m"(*value) : : "xmm0");
#endif
}
extern "C" UG2_SSE2 unsigned int __cdecl UG2FxCrtSseConvertInteger(const D3DXCrtSseValue *value) {
    unsigned int result;
#ifdef _MSC_VER
    __asm { mov eax,value
        movdqu xmm0,xmmword ptr [eax]
        cvtsd2si eax,xmm0
        mov result,eax }
#else
    asm volatile("movdqu %1,%%xmm0; cvtsd2si %%xmm0,%0" : "=r"(result) : "m"(*value) : "xmm0");
#endif
    return result;
}
