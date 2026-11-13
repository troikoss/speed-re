// Original UG2 PC v1.2 0x5e0920, 54 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_00440ef0();


void __fastcall FUN_005e0920(int param_1)

{
  int iVar1;
  int *piVar2;
  int *piVar3;
  
  if (((*(int **)(param_1 + 0x34) != (int *)0x0) && (iVar1 = **(int **)(param_1 + 0x34), iVar1 != 0)
      ) && (*(undefined4 **)(iVar1 + 0x34) != (undefined4 *)0x0)) {
    **(undefined4 **)(iVar1 + 0x34) = 0;
    *(undefined4 *)(iVar1 + 0x34) = 0;
  }
  piVar2 = *(int **)(param_1 + 0x30);
  iVar1 = *piVar2;
  piVar3 = (int *)piVar2[1];
  *piVar3 = iVar1;
  *(int **)(iVar1 + 4) = piVar3;
  FUN_00440ef0(piVar2);
  return;
}


