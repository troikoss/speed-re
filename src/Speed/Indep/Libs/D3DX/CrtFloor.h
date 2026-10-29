#ifndef UG2_D3DX_CRT_FLOOR_H
#define UG2_D3DX_CRT_FLOOR_H
#include "EffectVm.h"
struct D3DXCrtMathException {
    int type00;const char *name04;double input08,input10,result18;
};
typedef char D3DXCrtMathExceptionSize[(sizeof(D3DXCrtMathException)==32)?1:-1];
struct D3DXCrtMathName { unsigned int operation00;const char *name04; };
struct D3DXCrtDoubleBits { unsigned int low,high; };
extern "C" {
long double __cdecl D3DXCrtFloor(double); // 0x75eb50 / 0x769764
int __cdecl D3DXCrtRawControl(unsigned int,unsigned int); // 0x7676a4
long double __cdecl D3DXCrtRoundCurrent(double); // 0x76cc73
int __cdecl D3DXCrtSpecialType(double); // 0x766ea9
int *__cdecl D3DXCrtErrno(); // 0x7661ba
int __cdecl D3DXCrtSetMathErrno(int); // 0x767485
int __cdecl D3DXCrtDefaultMathError(D3DXCrtMathException *); // 0x727e6b
long double __cdecl D3DXCrtUserMathError(int,unsigned int,D3DXCrtDoubleBits,D3DXCrtDoubleBits,D3DXCrtDoubleBits,unsigned int); // 0x7674da
long double __cdecl D3DXCrtHandleQnan(unsigned int,double,unsigned int); // 0x767578
void __cdecl D3DXCrtSseMathError(double *,double *,double *,int); // 0x7694d0
// Instruction adapters and explicit platform/runtime boundaries.
unsigned int __cdecl UG2FxCrtReadMxcsr();
unsigned int __cdecl UG2FxCrtReadControlNoWait();
unsigned int __cdecl UG2FxCrtSseLess(const void *,const void *);
unsigned int __cdecl UG2FxCrtSseUnordered(const void *);
void __cdecl UG2FxCrtSseSubtract(void *,const void *,const void *);
void __cdecl UG2FxCrtSseTruncate(void *,const void *,const void *);
void __cdecl UG2FxCrtRoundStore(double *,const double *);
void *__cdecl UG2FxCrtThreadData();
int __cdecl UG2FxCrtCallMathError(unsigned int,D3DXCrtMathException *);
long double __cdecl UG2FxCrtHandleResult(unsigned int,unsigned int,D3DXCrtDoubleBits,D3DXCrtDoubleBits,unsigned int); // recovered 0x7675cb
extern unsigned int ug2CrtSseEnabled8BA480,ug2CrtFloorControl81AA60,ug2CrtMathErrorFlag81ACE0,ug2CrtMathErrorCallback81AA5C;
extern const unsigned int ug2CrtFloorShift7D3CA0[4],ug2CrtFloorExponentMask7D3CD0[4];
extern const double ug2CrtFloorOne7D3C90,ug2CrtFloorNegativeOne7D3CB0,ug2CrtFloorNegativeZero7D3CC0;
extern const D3DXCrtMathName ug2CrtMathNames81A838[29];
}
// Original UMATHERR copies these fields with integer MOVs, retaining SNaN bits.
#define UG2_CRT_COPY_DOUBLE_BITS(to,from) do { unsigned int *crtTo=reinterpret_cast<unsigned int *>(&(to));const unsigned int *crtFrom=reinterpret_cast<const unsigned int *>(&(from));crtTo[0]=crtFrom[0];crtTo[1]=crtFrom[1]; } while (0)
#endif
