#ifndef UG2_D3DX_EFFECT_VM_H
#define UG2_D3DX_EFFECT_VM_H
#include "EffectEvaluation.h"
extern "C" {
int __stdcall D3DXEffectVm(unsigned int *,unsigned int *,unsigned int,unsigned int,unsigned int,
    unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int); // 0x688e79
void __stdcall D3DXEffectVmOperation502(unsigned int,double **,double *); // 0x688ce2
// Unrecovered CRT math / operation dependencies. Ordinary source arguments
// replace the private original x87 input ABI; no host-math substitute is used.
long double __cdecl UG2FxVmFloor(double); // 0x75eb50
long double __cdecl UG2FxVmPower(double,double); // 0x75c9b0
long double __cdecl UG2FxVmAsin(double); // 0x75eea0
long double __cdecl UG2FxVmAcos(double); // 0x75db80
long double __stdcall UG2FxVmOperation63F2B5(float *); // 0x63f2b5
extern const double ug2VmZero786E60,ug2VmOne786E68,ug2VmTwo787AD0,ug2VmLogScale7BAA88;
}
#ifdef _MSC_VER
extern "C" double __cdecl sin(double),cos(double),atan(double),atan2(double,double),log(double),sqrt(double),fabs(double);
#pragma intrinsic(sin,cos,atan,atan2,log,sqrt,fabs)
#define UG2_FX_VM_SIN(x) sin(x)
#define UG2_FX_VM_COS(x) cos(x)
#define UG2_FX_VM_ATAN(x) atan(x)
#define UG2_FX_VM_ATAN2(a,b) atan2(a,b)
#define UG2_FX_VM_LOG(x) (static_cast<long double>(log(x))*ug2VmLogScale7BAA88)
#define UG2_FX_VM_RSQRT(x) (ug2VmOne786E68/sqrt(fabs(x)))
#define UG2_FX_VM_ADD(a,b) (static_cast<long double>(a)+static_cast<long double>(b))
#define UG2_FX_VM_MUL(a,b) (static_cast<long double>(a)*static_cast<long double>(b))
#define UG2_FX_VM_MADD(a,b,c) (static_cast<long double>(a)*static_cast<long double>(b)+static_cast<long double>(c))
#define UG2_FX_VM_RECIP(a) (static_cast<long double>(1.0)/static_cast<long double>(a))
#define UG2_FX_VM_LT(a,b) ((a)<(b))
#define UG2_FX_VM_GT(a,b) ((a)>(b))
#define UG2_FX_VM_GE(a,b) ((a)>=(b))
#define UG2_FX_VM_STORE_LOAD(to,a) do { volatile long double vmValue=(a);(to)=vmValue; } while (0)
#define UG2_FX_VM_STORE_NEG(to,a) do { volatile long double vmValue=(a);(to)=-vmValue; } while (0)
#define UG2_FX_VM_STORE_SIN(to,a) ((to)=UG2_FX_VM_SIN(a))
#define UG2_FX_VM_STORE_COS(to,a) ((to)=UG2_FX_VM_COS(a))
#define UG2_FX_VM_STORE_ATAN(to,a) ((to)=UG2_FX_VM_ATAN(a))
#define UG2_FX_VM_STORE_ATAN2(to,a,b) ((to)=UG2_FX_VM_ATAN2(a,b))
#define UG2_FX_VM_STORE_LOG(to,a) ((to)=UG2_FX_VM_LOG(a))
#define UG2_FX_VM_STORE_RSQRT(to,a) ((to)=UG2_FX_VM_RSQRT(a))
#define UG2_FX_VM_STORE_ADD(to,a,b) ((to)=UG2_FX_VM_ADD(a,b))
#define UG2_FX_VM_STORE_MUL(to,a,b) ((to)=UG2_FX_VM_MUL(a,b))
#define UG2_FX_VM_STORE_MADD(to,a,b,c) ((to)=UG2_FX_VM_MADD(a,b,c))
#define UG2_FX_VM_STORE_RECIP(to,a) ((to)=UG2_FX_VM_RECIP(a))
#define UG2_FX_VM_STORE_FRACTION(to,a,b) ((to)=static_cast<long double>(a)-(b))
#endif
#endif
