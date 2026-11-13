// Original UG2 PC v1.2 0x739eb0, 44 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0045a4c0();
#define DAT_008b7bc4 (*(undefined4 *)0x008b7bc4)

void FUN_00739eb0(int param_1,undefined1 param_2,undefined1 param_3)

{
  undefined4 uVar1;
  
  *(undefined4 *)(param_1 + 0x10) = DAT_008b7bc4;
  *(undefined1 *)(param_1 + 0x15) = param_3;
  uVar1 = FUN_0045a4c0(*(int *)(param_1 + 0x54));
  *(char *)(param_1 + 0x14) = (char)uVar1;
  *(undefined1 *)(param_1 + 0x16) = param_2;
  return;
}


