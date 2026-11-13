// Original UG2 PC v1.2 0x742c60, 23 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_00742e10();
int __cdecl FUN_0074ad10();
#define DAT_008b7bd4 (*(undefined4 *)0x008b7bd4)

undefined4 FUN_00742c60(void)

{
  FUN_00742e10();
  FUN_0074ad10();
  DAT_008b7bd4 = 0;
  return 0;
}


