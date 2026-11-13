// Original UG2 PC v1.2 0x73f810, 70 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0073f220();
int __cdecl FUN_0074db20();
#define DAT_00783fe5 (*(undefined4 *)0x00783fe5)

undefined4 FUN_0073f810(int param_1)

{
  *(undefined4 *)(param_1 + 0x14) = 5;
  *(undefined4 *)(param_1 + 100) = 0;
  if (*(int *)(param_1 + 0xb0) == 0) {
    FUN_0074db20(*(int *)(param_1 + 8),&DAT_00783fe5,5,0);
  }
  FUN_0073f220(param_1);
  *(undefined4 *)(param_1 + 0xc) = 5;
  return 0;
}


