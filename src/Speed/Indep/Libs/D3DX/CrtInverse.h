#ifndef UG2_D3DX_CRT_INVERSE_H
#define UG2_D3DX_CRT_INVERSE_H
#include "CrtExceptions.h"
// A storage view of x87 extended precision, not a new numeric implementation.
// Only the first ten bytes carry a value; padding is not compared as evidence.
struct D3DXCrtExtended { unsigned int low,high;unsigned short exponent,padding; };
extern "C" {
void __cdecl D3DXCrtAsin80(D3DXCrtExtended *,const D3DXCrtExtended *); // 0x75eea0 / 0x75eebd
void __cdecl D3DXCrtAcos80(D3DXCrtExtended *,const D3DXCrtExtended *); // 0x75db80 / 0x75db9d
void __cdecl D3DXCrtAsinCore80(D3DXCrtExtended *,const D3DXCrtExtended *,D3DXCrtDoubleBits,unsigned int,unsigned int); // 0x75eebd
void __cdecl D3DXCrtAcosCore80(D3DXCrtExtended *,const D3DXCrtExtended *,D3DXCrtDoubleBits,unsigned int,unsigned int); // 0x75db9d
unsigned int __cdecl D3DXCrtInspectOperand(D3DXCrtDoubleBits); // 0x765748, plus its ZF classification at callers
void __cdecl D3DXCrtPrepareControl(unsigned int); // 0x7656d5
unsigned int __cdecl D3DXCrtPrepareNan(D3DXCrtExtended *,unsigned int); // 0x7656ec
void __cdecl D3DXCrtOneArgError(unsigned int,unsigned int,const char *,D3DXCrtDoubleBits,unsigned int,D3DXCrtExtended *); // 0x765677
void __cdecl D3DXCrtQuickExit(unsigned int); // 0x76575e
void __cdecl D3DXCrtMathExit(unsigned int,const char *,D3DXCrtDoubleBits,unsigned int,D3DXCrtExtended *); // 0x76576b
void __cdecl D3DXCrtOneArgDispatch(unsigned int,D3DXCrtMathException *,unsigned short *); // 0x76cb75
// x87 instruction adapters and the public CDECL/private extended-return bridge.
void __cdecl UG2FxCrtExtendedFromDouble(D3DXCrtExtended *,const double *);
void __cdecl UG2FxCrtExtendedToDouble(double *,const D3DXCrtExtended *);
void __cdecl UG2FxCrtExtendedAddDouble(D3DXCrtExtended *,const double *);
void __cdecl UG2FxCrtExtendedAddOne(D3DXCrtExtended *,const D3DXCrtExtended *);
void __cdecl UG2FxCrtExtendedOneMinus(D3DXCrtExtended *,const D3DXCrtExtended *);
void __cdecl UG2FxCrtExtendedMultiply(D3DXCrtExtended *,const D3DXCrtExtended *,const D3DXCrtExtended *);
void __cdecl UG2FxCrtExtendedSqrt(D3DXCrtExtended *);
void __cdecl UG2FxCrtExtendedAtan2(D3DXCrtExtended *,const D3DXCrtExtended *,const D3DXCrtExtended *);
void __cdecl UG2FxCrtExtendedPi(D3DXCrtExtended *);
long double __cdecl UG2FxCrtReturnExtended(const D3DXCrtExtended *);
extern unsigned int ug2CrtBypassMathExit8B8424;
extern const D3DXCrtExtended ug2CrtIndefinite81A4D0,ug2CrtHalfPi81A4DA;
extern const double ug2CrtNanAdd7D7C40;
}
#endif
