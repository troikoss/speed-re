// Original UG2 PC v1.2 0x61c5b0, 79 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_00438140();
int __cdecl FUN_00571060();
int __cdecl FUN_0057ef60();
int __cdecl FUN_00580e90();


void __fastcall FUN_0061c5b0(int param_1)

{
  int iVar1;
  
  if ((*(int *)(param_1 + 0x60) != 0) &&
     (iVar1 = *(int *)(*(int *)(param_1 + 0x60) + 0x54), iVar1 != 0)) {
    FUN_00580e90(iVar1);
    *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x54) = 0;
    *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x5c) = 0;
    FUN_00438140(0);
  }
  iVar1 = FUN_00571060();
  while (iVar1 == 0) {
    FUN_0057ef60();
    iVar1 = FUN_00571060();
  }
  return;
}


