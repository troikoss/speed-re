// Original UG2 PC v1.2 0x574830, 51 bytes. Auto-matched: byte-identical to speed2.exe under
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

#define _DAT_00787adc (*(float *)0x00787adc)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00574830(float *param_1)

{
  float fVar1;
  
  fVar1 = param_1[2] - *param_1;
  param_1[6] = param_1[1];
  param_1[7] = *param_1;
  param_1[4] = (param_1[1] + param_1[3]) - (fVar1 + fVar1);
  param_1[5] = (fVar1 * _DAT_00787adc - param_1[3]) - (param_1[1] + param_1[1]);
  return;
}


