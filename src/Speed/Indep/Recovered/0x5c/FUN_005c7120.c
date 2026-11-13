// Original UG2 PC v1.2 0x5c7120, 28 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_005c5f70();
#define DAT_007ff190 (*(undefined4 *)0x007ff190)

void FUN_005c7120(undefined4 param_1,undefined4 param_2)

{
  FUN_005c5f70(param_1,param_2,1,&DAT_007ff190,0);
  return;
}


