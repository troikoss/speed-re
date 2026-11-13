// Original UG2 PC v1.2 0x54e6b0, 33 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_004954e0();
int __stdcall FUN_00542990(int);


void __fastcall FUN_0054e6b0(int param_1)

{
  *(undefined1 *)(param_1 + 0x6c) = 0;
  FUN_00542990(0);
  FUN_004954e0(*(undefined4 *)(param_1 + 4),0x2bac0cee);
  return;
}


