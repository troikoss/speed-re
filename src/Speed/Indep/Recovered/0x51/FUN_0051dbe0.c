// Original UG2 PC v1.2 0x51dbe0, 69 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0050c960();
int __cdecl FUN_0051cf70();


void __fastcall FUN_0051dbe0(int param_1,int _edx_unused,undefined4 param_2)

{
  if (*(int *)(param_1 + 0x10) != 0) {
    FUN_0051cf70(*(int *)(param_1 + 0x10),param_2,1);
  }
  if (*(int *)(param_1 + 0x14) != 0) {
    FUN_0051cf70(*(int *)(param_1 + 0x14),param_2,1);
  }
  FUN_0050c960(param_1 + 0x20,0x100,param_2);
  return;
}


