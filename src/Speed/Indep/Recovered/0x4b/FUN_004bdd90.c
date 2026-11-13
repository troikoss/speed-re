// Original UG2 PC v1.2 0x4bdd90, 23 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_004bdc30();


void __fastcall FUN_004bdd90(int param_1)

{
  if (0xc < *(int *)(param_1 + 0x1178)) {
    *(int *)(param_1 + 0x1178) = *(int *)(param_1 + 0x1178) + -1;
  }
  FUN_004bdc30();
  return;
}


