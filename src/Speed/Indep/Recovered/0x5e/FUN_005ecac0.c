// Original UG2 PC v1.2 0x5ecac0, 223 bytes. Auto-matched: byte-identical to speed2.exe under
// MSVC 7.1 (/Ox /Ob0 dat=float), call targets masked. Source is Ghidra decompiler output with an inferred
// prelude; names, types and callee conventions are NOT verified beyond the bytes.
// Do not build into ug2recovered: globals are absolute-address macros.

typedef unsigned char undefined, undefined1, byte, uchar, bool8;
typedef unsigned short undefined2, ushort, word;
typedef unsigned int undefined4, uint, dword;
typedef unsigned __int64 undefined8, ulonglong;
typedef __int64 longlong;
typedef unsigned char bool;
#define true 1
#define false 0
typedef void (*code)();
typedef double float10;
typedef unsigned short wchar_t16;

#define _DAT_00784250 (*(float *)0x00784250)
#define _DAT_007a5654 (*(float *)0x007a5654)
#define _DAT_007a5658 (*(float *)0x007a5658)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_005ecac0(int param_1,int _edx_unused,float param_2)

{
  float fVar1;
  float fVar2;
  
  fVar1 = _DAT_00784250 / _DAT_007a5654;
  fVar2 = (*(float *)(param_1 + 0x330) - *(float *)(param_1 + 0x4f8)) * _DAT_007a5658 * fVar1 *
          param_2 + *(float *)(param_1 + 0x4f8);
  *(float *)(param_1 + 0x4f8) = fVar2;
  *(float *)(param_1 + 0x4e8) = fVar2 * param_2 + *(float *)(param_1 + 0x4e8);
  fVar2 = (*(float *)(param_1 + 0x334) - *(float *)(param_1 + 0x4fc)) * _DAT_007a5658 * fVar1 *
          param_2 + *(float *)(param_1 + 0x4fc);
  *(float *)(param_1 + 0x4fc) = fVar2;
  *(float *)(param_1 + 0x4ec) = fVar2 * param_2 + *(float *)(param_1 + 0x4ec);
  fVar2 = (*(float *)(param_1 + 0x338) - *(float *)(param_1 + 0x500)) * _DAT_007a5658 * fVar1 *
          param_2 + *(float *)(param_1 + 0x500);
  *(float *)(param_1 + 0x500) = fVar2;
  *(float *)(param_1 + 0x4f0) = fVar2 * param_2 + *(float *)(param_1 + 0x4f0);
  fVar1 = (*(float *)(param_1 + 0x33c) - *(float *)(param_1 + 0x504)) * _DAT_007a5658 * fVar1 *
          param_2 + *(float *)(param_1 + 0x504);
  *(float *)(param_1 + 0x504) = fVar1;
  *(float *)(param_1 + 0x4f4) = fVar1 * param_2 + *(float *)(param_1 + 0x4f4);
  return;
}


