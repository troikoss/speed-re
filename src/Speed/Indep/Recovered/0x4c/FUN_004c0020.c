// Original UG2 PC v1.2 0x4c0020, 33 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_00537980();


void __fastcall FUN_004c0020(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = *(int *)(param_1 + 0x38);
  if (*(int *)(param_1 + 0x30) != iVar1) {
    uVar2 = *(undefined4 *)(iVar1 + 8);
    iVar1 = *(int *)(iVar1 + 4);
    *(int *)(param_1 + 0x38) = iVar1;
    FUN_00537980(uVar2,*(undefined4 *)(iVar1 + 8),0);
  }
  return;
}


