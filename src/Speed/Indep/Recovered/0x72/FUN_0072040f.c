// Original UG2 PC v1.2 0x72040f, 37 bytes. Auto-matched: byte-identical to speed2.exe under
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

#define _DAT_008b4054 (*(float *)0x008b4054)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0072040f(int param_1)

{
  float fVar1;
  
  fVar1 = *(float *)(param_1 + 0x18 + (uint)*(byte *)(param_1 + 3) * 8);
  *(float *)(param_1 + 4) = fVar1;
  fVar1 = (*(float *)(param_1 + 0x1c + (uint)*(byte *)(param_1 + 3) * 8) - *(float *)(param_1 + 0xc)
          ) / fVar1;
  *(float *)(param_1 + 8) = fVar1;
  *(float *)(param_1 + 8) = fVar1 * _DAT_008b4054;
  return;
}


