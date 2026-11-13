// Original UG2 PC v1.2 0x61cfe0, 108 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_00610cd0();
#define DAT_00803934 (*(float *)0x00803934)
#define _DAT_0078425c (*(float *)0x0078425c)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0061cfe0(int param_1,int _edx_unused,int param_2)

{
  *(float *)(param_1 + 0x20) = (float)(&DAT_00803934)[param_2] - _DAT_0078425c;
  *(float *)(param_1 + 0x24) = (float)(&DAT_00803934)[param_2] - _DAT_0078425c;
  *(float *)(param_1 + 0x28) = (float)(&DAT_00803934)[param_2] - _DAT_0078425c;
  *(float *)(param_1 + 0x2c) = (float)(&DAT_00803934)[param_2] - _DAT_0078425c;
  *(float *)(param_1 + 0x30) = (float)(&DAT_00803934)[param_2] - _DAT_0078425c;
  *(float *)(param_1 + 0x34) = (float)(&DAT_00803934)[param_2] - _DAT_0078425c;
  FUN_00610cd0();
  return;
}


