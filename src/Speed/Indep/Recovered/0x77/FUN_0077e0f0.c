// Original UG2 PC v1.2 0x77e0f0, 21 bytes. Auto-matched: byte-identical to speed2.exe under
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

#define _DAT_0086b3c8 (*(undefined4 *)0x0086b3c8)
#define DAT_0086b5b0 (*(undefined4 *)0x0086b5b0)
#define _DAT_0086b3cc (*(undefined4 *)0x0086b3cc)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0077e0f0(void)

{
  _DAT_0086b3c8 = &DAT_0086b5b0;
  _DAT_0086b3cc = 7;
  return;
}


