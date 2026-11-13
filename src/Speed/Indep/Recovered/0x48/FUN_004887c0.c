// Original UG2 PC v1.2 0x4887c0, 63 bytes. Auto-matched: byte-identical to speed2.exe under
// MSVC 7.1 (/Ox /Ob0 dat=undefined4), call targets masked. Source is Ghidra decompiler output with an inferred
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
int __cdecl FUN_005b98b0();
#define DAT_0082daac (*(undefined4 *)0x0082daac)
#define DAT_0082dab0 (*(undefined4 *)0x0082dab0)
#define DAT_0082dab4 (*(undefined4 *)0x0082dab4)
#define _DAT_0082dab8 (*(undefined4 *)0x0082dab8)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_004887c0(void)

{
  if ((DAT_0082daac == 0) && ((DAT_0082dab0 != 0 || (DAT_0082dab4 != 0)))) {
    FUN_005b98b0();
    DAT_0082dab0 = 0;
    DAT_0082dab4 = 0;
    _DAT_0082dab8 = 0;
  }
  return;
}


