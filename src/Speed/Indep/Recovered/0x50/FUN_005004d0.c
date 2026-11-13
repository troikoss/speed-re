// Original UG2 PC v1.2 0x5004d0, 78 bytes. Auto-matched: byte-identical to speed2.exe under
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



int __fastcall FUN_005004d0(int param_1)

{
  int iVar1;
  int *piVar2;
  int iVar3;
  
  iVar1 = 0;
  piVar2 = (int *)(param_1 + 8);
  iVar3 = 5;
  do {
    if (piVar2[-2] != 0) {
      iVar1 = iVar1 + 1;
    }
    if (piVar2[-1] != 0) {
      iVar1 = iVar1 + 1;
    }
    if (*piVar2 != 0) {
      iVar1 = iVar1 + 1;
    }
    if (piVar2[1] != 0) {
      iVar1 = iVar1 + 1;
    }
    if (piVar2[2] != 0) {
      iVar1 = iVar1 + 1;
    }
    if (piVar2[3] != 0) {
      iVar1 = iVar1 + 1;
    }
    if (piVar2[4] != 0) {
      iVar1 = iVar1 + 1;
    }
    piVar2 = piVar2 + 7;
    iVar3 = iVar3 + -1;
  } while (iVar3 != 0);
  return iVar1;
}


