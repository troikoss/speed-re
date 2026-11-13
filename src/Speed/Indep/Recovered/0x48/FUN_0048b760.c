// Original UG2 PC v1.2 0x48b760, 48 bytes. Auto-matched: byte-identical to speed2.exe under
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

#define _DAT_0082db1c (*(undefined4 *)0x0082db1c)
#define _DAT_00830d90 (*(undefined4 *)0x00830d90)
#define _DAT_00830d94 (*(undefined4 *)0x00830d94)
#define _DAT_00830d98 (*(undefined4 *)0x00830d98)
#define _DAT_0082d5a8 (*(undefined4 *)0x0082d5a8)
#define _DAT_0082d5ac (*(undefined4 *)0x0082d5ac)
#define _DAT_0082d5b0 (*(undefined4 *)0x0082d5b0)
#define _DAT_0082d5b4 (*(undefined4 *)0x0082d5b4)
#define _DAT_0082d5b8 (*(undefined4 *)0x0082d5b8)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0048b760(void)

{
  _DAT_0082db1c = 0;
  _DAT_00830d90 = 0;
  _DAT_00830d94 = 0;
  _DAT_00830d98 = 0;
  _DAT_0082d5a8 = 0;
  _DAT_0082d5ac = 0;
  _DAT_0082d5b0 = 0;
  _DAT_0082d5b4 = 0;
  _DAT_0082d5b8 = 0;
  return;
}


