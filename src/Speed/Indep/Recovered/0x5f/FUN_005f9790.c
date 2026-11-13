// Original UG2 PC v1.2 0x5f9790, 58 bytes. Auto-matched: byte-identical to speed2.exe under
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



int __fastcall FUN_005f9790(int param_1)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  int *piVar4;
  int iVar5;
  
  iVar3 = 0;
  piVar4 = (int *)(param_1 + 8);
  iVar5 = 6;
  do {
    piVar1 = (int *)*piVar4;
    if ((((piVar1 != (int *)0x0) && (piVar1 != *(int **)(param_1 + 4))) &&
        (iVar2 = *piVar1, iVar2 != 4)) && (((iVar2 != 6 && (iVar2 != 0)) && (iVar2 != 5)))) {
      iVar3 = iVar3 + 1;
    }
    piVar4 = piVar4 + 1;
    iVar5 = iVar5 + -1;
  } while (iVar5 != 0);
  return iVar3;
}


