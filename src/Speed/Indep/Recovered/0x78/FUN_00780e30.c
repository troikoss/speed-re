// Original UG2 PC v1.2 0x780e30, 15 bytes. Auto-matched: byte-identical to speed2.exe under
// MSVC 7.1 (/Ox /Ob0 dat=double), call targets masked. Source is Ghidra decompiler output with an inferred
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

#define _DAT_008ab518 (*(double *)0x008ab518)
#define _DAT_007c1d88 (*(double *)0x007c1d88)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00780e30(void)

{
  _DAT_008ab518 = _DAT_007c1d88 + _DAT_007c1d88;
  return;
}


