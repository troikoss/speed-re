#ifndef UG2_NATIVE_FX_VM_INTRINSICS_H
#define UG2_NATIVE_FX_VM_INTRINSICS_H
// Native x86 scaffolding for the original compiler's inline x87 intrinsics.
// These are instruction semantics (including range/status/NaN behavior), not
// host libm implementations or register choices intended to match code bytes.
// The original FCOMP raises invalid for quiet as well as signaling NaNs.
#define UG2_FX_VM_COMPARE(a,b) ({ unsigned short vmStatus;asm volatile("fldl %1; fcompl %2; fnstsw %0" : "=am"(vmStatus) : "m"(a),"m"(b) : "st");vmStatus; })
#define UG2_FX_VM_LT(a,b) ((UG2_FX_VM_COMPARE(a,b)&0x500)==0x100)
#define UG2_FX_VM_GT(a,b) (!(UG2_FX_VM_COMPARE(a,b)&0x4100))
#define UG2_FX_VM_GE(a,b) (!(UG2_FX_VM_COMPARE(a,b)&0x100))
#define UG2_FX_VM_EQ(a,b) ((UG2_FX_VM_COMPARE(a,b)&0x4400)==0x4000)
// Store directly to the original destination. A compiler-generated double
// spill/reload can add a denormal exception after a newly underflowed result.
#define UG2_FX_VM_STORE_UNARY(to,a,instructions) do { asm volatile("fldl %1; " instructions "; fstpl %0" : "=m"(to) : "m"(a) : "st"); } while (0)
#define UG2_FX_VM_STORE_LOAD(to,a) UG2_FX_VM_STORE_UNARY(to,a,"")
#define UG2_FX_VM_STORE_NEG(to,a) UG2_FX_VM_STORE_UNARY(to,a,"fchs")
#define UG2_FX_VM_STORE_SIN(to,a) UG2_FX_VM_STORE_UNARY(to,a,"fsin")
#define UG2_FX_VM_STORE_COS(to,a) UG2_FX_VM_STORE_UNARY(to,a,"fcos")
#define UG2_FX_VM_STORE_ATAN(to,a) UG2_FX_VM_STORE_UNARY(to,a,"fld1; fpatan")
#define UG2_FX_VM_STORE_ATAN2(to,a,b) do { asm volatile("fldl %1; fldl %2; fpatan; fstpl %0" : "=m"(to) : "m"(a),"m"(b) : "st"); } while (0)
#define UG2_FX_VM_STORE_LOG(to,a) do { asm volatile("fldl %1; fldln2; fxch; fyl2x; fmull %2; fstpl %0" : "=m"(to) : "m"(a),"m"(ug2VmLogScale7BAA88) : "st"); } while (0)
#define UG2_FX_VM_STORE_RSQRT(to,a) do { asm volatile("fldl %1; fabs; fsqrt; fdivrl %2; fstpl %0" : "=m"(to) : "m"(a),"m"(ug2VmOne786E68) : "st"); } while (0)
#define UG2_FX_VM_STORE_BINARY(to,a,b,instruction) do { asm volatile("fldl %1; " instruction " %2; fstpl %0" : "=m"(to) : "m"(a),"m"(b) : "st"); } while (0)
#define UG2_FX_VM_STORE_ADD(to,a,b) UG2_FX_VM_STORE_BINARY(to,a,b,"faddl")
#define UG2_FX_VM_STORE_MUL(to,a,b) UG2_FX_VM_STORE_BINARY(to,a,b,"fmull")
#define UG2_FX_VM_STORE_MADD(to,a,b,c) do { asm volatile("fldl %1; fmull %2; faddl %3; fstpl %0" : "=m"(to) : "m"(a),"m"(b),"m"(c) : "st"); } while (0)
#define UG2_FX_VM_STORE_RECIP(to,a) do { asm volatile("fld1; fdivl %1; fstpl %0" : "=m"(to) : "m"(a) : "st"); } while (0)
#define UG2_FX_VM_STORE_FRACTION(to,a,value) do { asm volatile("fldt %2; fsubrl %1; fstpl %0" : "=m"(to) : "m"(a),"m"(value) : "st"); } while (0)
#endif
