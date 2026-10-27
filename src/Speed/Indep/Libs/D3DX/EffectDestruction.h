#ifndef UG2_D3DX_EFFECT_DESTRUCTION_H
#define UG2_D3DX_EFFECT_DESTRUCTION_H
#include "EffectFactory.h"

// Names describe only the observed cleanup layouts, not inferred D3DX types.
struct D3DXEffectBaseRecord {
    unsigned int value00,value04,value08,value0C,value10,value14,value18,value1C;
    D3DXEffectBaseRecord *array20,*linked24;
    D3DXEffectBaseRecord *Delete(unsigned char flags); // 0x65b6a8
    D3DXEffectBaseRecord *DeleteArray(unsigned char flags); // 0x661405
};
struct D3DXEffectReference08 {
    int value00;
    unsigned int value04;
    D3DXEffectReference08 *DeleteArray(unsigned char flags); // 0x66238c
};
struct D3DXEffectReference18 {
    int value00;
    unsigned int value04,value08,value0C,value10,value14;
    D3DXEffectReference18 *DeleteArray(unsigned char flags); // 0x661453
};
struct D3DXEffectRecord20 {
    unsigned int unknown00[8];
    D3DXEffectRecord20 *DeleteArray(unsigned char flags); // 0x660f55
};
struct D3DXEffectRecord64 {
    unsigned int value00,value04;
    D3DXEffectBaseRecord value08;
    unsigned int value30;
    D3DXEffectRecord20 *array34;
    unsigned int value38,value3C,value40,value44,value48;
    unsigned short value4C,value4E,value50,value52,value54,value56;
    unsigned int value58,value5C,value60;
    D3DXEffectRecord64 *DeleteArray(unsigned char flags); // 0x664f63
};
struct D3DXEffectGroup64 {
    void *data00;
    D3DXEffectRecord64 *array04;
    unsigned int value08,value0C,value10,value14;
    unsigned short value18,value1A;
    unsigned int value1C,value20,value24,value28,value2C,value30;
    unsigned short value34,value36,value38,value3A,value3C,value3E;
    unsigned int value40,value44,value48,value4C,value50,value54,value58,value5C,value60;
    D3DXEffectGroup64 *DeleteArray(unsigned char flags); // 0x66a4d5
    int ResizeRecords(unsigned short count); // 0x664fb1
};
struct D3DXEffectRecord38 {
    D3DXEffectBaseRecord value00;
    unsigned int value28,value2C;
    D3DXEffectParameter *parameter30;
    unsigned int value34;
    D3DXEffectRecord38 *DeleteArray(unsigned char flags); // 0x665050
};
struct D3DXEffectRecordB4 {
    D3DXEffectBaseRecord value00;
    unsigned int value28,value2C;
    D3DXEffectParameter *parameter30;
    unsigned int value34;
    D3DXEffectRecord38 *array38;
    unsigned int value3C;
    void *data40;
    unsigned int unknown44[17],value88,value8C[4],unknown9C[6];
    D3DXEffectRecordB4 *DeleteArray(unsigned char flags); // 0x66a523
};
typedef char D3DXEffectBaseRecordSize[(sizeof(D3DXEffectBaseRecord)==0x28)?1:-1];
typedef char D3DXEffectReference08Size[(sizeof(D3DXEffectReference08)==8)?1:-1];
typedef char D3DXEffectReference18Size[(sizeof(D3DXEffectReference18)==0x18)?1:-1];
typedef char D3DXEffectRecord20Size[(sizeof(D3DXEffectRecord20)==0x20)?1:-1];
typedef char D3DXEffectRecord64Size[(sizeof(D3DXEffectRecord64)==0x64)?1:-1];
typedef char D3DXEffectGroup64Size[(sizeof(D3DXEffectGroup64)==0x64)?1:-1];
typedef char D3DXEffectRecord38Size[(sizeof(D3DXEffectRecord38)==0x38)?1:-1];
typedef char D3DXEffectRecordB4Size[(sizeof(D3DXEffectRecordB4)==0xb4)?1:-1];

// One-argument fastcall has the same ECX receiver / no-stack-argument ABI as
// these original thiscall callbacks; it also permits a typed callback pointer.
typedef void (__fastcall *D3DXEffectDestructor)(void *);
extern "C" {
void __stdcall D3DXEffectDestroyArray(void *,unsigned int,unsigned int,D3DXEffectDestructor); // 0x64b5ff
void __fastcall D3DXDestroyEmpty(void *); // 0x6996f4
void __fastcall D3DXDestroyBase(void *); // 0x663e90
void __fastcall D3DXDestroyParameter(void *); // 0x664efa
void __fastcall D3DXDestroyValue(void *); // 0x663fcf
void __fastcall D3DXDestroyReference08(void *); // 0x65ee40
void __fastcall D3DXDestroyReference18(void *); // 0x65b7ad
void __fastcall D3DXDestroyRecord64(void *); // 0x663f15
void __fastcall D3DXDestroyGroup64(void *); // 0x66a419
void __fastcall D3DXDestroyRecord38(void *); // 0x665031
void __fastcall D3DXDestroyRecordB4(void *); // 0x66a442
void __fastcall D3DXDestroySecondary(void *); // 0x65e4ff; receiver is effect+4
void __fastcall D3DXDestroyTemporaries(void *); // 0x65b7f0
void __fastcall D3DXDestroyEffect(void *); // 0x66adec
unsigned int __stdcall UG2FxComRelease(unsigned int object); // vtable +8 boundary
}
#endif
