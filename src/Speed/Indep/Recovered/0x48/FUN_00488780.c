// Original UG2 PC v1.2 0x488780, 49 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0043cd20();
int __cdecl FUN_0043cd30();
int __cdecl FUN_00440e40();
#define DAT_0082da38 (*(undefined4 *)0x0082da38)

void FUN_00488780(void)

{
  FUN_0043cd20(DAT_0082da38);
  FUN_0043cd30(DAT_0082da38);
  FUN_00440e40(DAT_0082da38);
  DAT_0082da38 = 0;
  return;
}


