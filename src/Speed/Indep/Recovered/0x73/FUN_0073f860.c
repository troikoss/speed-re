// Original UG2 PC v1.2 0x73f860, 97 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0073f2e0();
int __cdecl FUN_0074d1b0();
int __cdecl FUN_0074d3b0();
int __cdecl FUN_0074db20();


undefined4 FUN_0073f860(int param_1)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 0x38) != 2) {
    return 0;
  }
  *(undefined4 *)(param_1 + 0x10) = 0;
  *(undefined4 *)(param_1 + 0x14) = 1;
  FUN_0074d1b0(*(int *)(param_1 + 8),FUN_0073f2e0,param_1);
  if (*(int *)(param_1 + 0xb0) == 0) {
    FUN_0074db20(*(int *)(param_1 + 8),(byte *)0x0,*(int *)(param_1 + 0x14),0);
  }
  *(undefined4 *)(param_1 + 0xc) = 1;
  uVar1 = FUN_0074d3b0(*(int *)(param_1 + 8));
  return uVar1;
}


