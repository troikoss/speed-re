// Original UG2 PC v1.2 0x5efa90, 27 bytes. Auto-matched: byte-identical to speed2.exe under
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

#define _DAT_00784268 (*(float *)0x00784268)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 __fastcall FUN_005efa90(int param_1,int _edx_unused,undefined4 *param_2)

{
  *param_2 = *(undefined4 *)(param_1 + 0x280);
  return (float10)*(int *)(param_1 + 0x278) * (float10)_DAT_00784268;
}


