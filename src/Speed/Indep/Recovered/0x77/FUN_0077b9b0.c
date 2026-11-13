// Original UG2 PC v1.2 0x77b9b0, 11 bytes. Auto-matched: byte-identical to speed2.exe under
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

#define _DAT_0082a4ec (*(undefined4 *)0x0082a4ec)
#define DAT_0082a4e8 (*(undefined4 *)0x0082a4e8)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0077b9b0(void)

{
  _DAT_0082a4ec = DAT_0082a4e8;
  return;
}


