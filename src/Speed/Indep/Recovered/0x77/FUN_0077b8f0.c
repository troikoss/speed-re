// Original UG2 PC v1.2 0x77b8f0, 37 bytes. Auto-matched: byte-identical to speed2.exe under
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

#define DAT_0082a4e8 (*(float *)0x0082a4e8)
#define _DAT_0082a458 (*(float *)0x0082a458)
#define _DAT_00786ff0 (*(float *)0x00786ff0)
#define _DAT_00784560 (*(float *)0x00784560)
#define _DAT_007844c0 (*(float *)0x007844c0)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0077b8f0(void)

{
  DAT_0082a4e8 = (((_DAT_0082a458 - _DAT_00786ff0) - _DAT_00784560) - _DAT_007844c0) - _DAT_007844c0
  ;
  return;
}


