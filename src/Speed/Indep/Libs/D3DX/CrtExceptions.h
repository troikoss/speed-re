#ifndef UG2_D3DX_CRT_EXCEPTIONS_H
#define UG2_D3DX_CRT_EXCEPTIONS_H
#include "CrtFloor.h"
extern "C" {
long double __cdecl D3DXCrtHandleResult(unsigned int,unsigned int,D3DXCrtDoubleBits,D3DXCrtDoubleBits,unsigned int); // 0x7675cb
int __cdecl D3DXCrtApplyMasks(unsigned int,double *,unsigned int); // 0x767263
void __cdecl D3DXCrtRaiseException(unsigned int *,unsigned int *,unsigned int,unsigned int,double *,double *); // 0x766fbe
long double __cdecl D3DXCrtDecompose(D3DXCrtDoubleBits,int *); // 0x766f04
long double __cdecl D3DXCrtSetExponent(D3DXCrtDoubleBits,int); // 0x766e7f
int __cdecl D3DXCrtErrorCode(unsigned int); // 0x7674ad
void __cdecl D3DXCrtSetStatus(unsigned int); // 0x7676c8
int __cdecl D3DXCrtReadStatus(); // 0x767689
int __cdecl D3DXCrtClearStatus(); // 0x767696
unsigned int __cdecl UG2FxCrtReadStatusWait();
unsigned int __cdecl UG2FxCrtClearStatusNoWait();
void __cdecl UG2FxCrtRaiseInvalid();
void __cdecl UG2FxCrtRaiseOverflow();
void __cdecl UG2FxCrtRaiseUnderflow();
void __cdecl UG2FxCrtRaiseZeroDivide();
void __cdecl UG2FxCrtRaisePrecision();
void __stdcall UG2FxCrtOsRaiseException(unsigned int,unsigned int,unsigned int,unsigned int *);
extern const double ug2CrtInfinity81A810,ug2CrtMaximum81A820;
extern const unsigned char ug2CrtRaiseLarge81A920[12],ug2CrtRaiseTiny81A92C[12];
}
#endif
