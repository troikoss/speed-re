// Original UG2 PC v1.2 0x43cb20, 30 bytes. Auto-matched: byte-identical to speed2.exe under
// MSVC 7.1 (/Ox /Ob0 dat=undefined1), call targets masked. Source is Ghidra decompiler output with an inferred
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

#define DAT_007eb3f0 (*(undefined1 *)0x007eb3f0)
#define DAT_007eb3f1 (*(undefined1 *)0x007eb3f1)
#define DAT_007eb3f2 (*(undefined1 *)0x007eb3f2)

void FUN_0043cb20(undefined1 param_1,undefined1 param_2,undefined1 param_3)

{
  DAT_007eb3f0 = param_1;
  DAT_007eb3f1 = param_2;
  DAT_007eb3f2 = param_3;
  return;
}


