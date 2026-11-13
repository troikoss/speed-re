// Original UG2 PC v1.2 0x780470, 15 bytes. Auto-matched: byte-identical to speed2.exe under
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

#define _DAT_008aaddc (*(float *)0x008aaddc)
#define _DAT_007c1348 (*(float *)0x007c1348)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00780470(void)

{
  _DAT_008aaddc = _DAT_007c1348 + _DAT_007c1348;
  return;
}


