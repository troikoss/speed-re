// Original UG2 PC v1.2 0x5b8460, 239 bytes. Auto-matched: byte-identical to speed2.exe under
// MSVC 7.1 (/Ox /Ob0 dat=undefined1), call targets masked. Source is Ghidra decompiler output with an inferred
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

#define DAT_00870768 (*(undefined1 *)0x00870768)

void __fastcall FUN_005b8460(int param_1,int _edx_unused,int param_2)

{
  int iVar1;
  bool bVar2;
  int *piVar3;
  int iVar4;
  
  bVar2 = true;
  if ((*(char *)(param_1 + 0x364) != '\0') &&
     ((*(int *)(param_1 + 0x33c) == 3 || (*(int *)(param_1 + 0x33c) == 4)))) {
    bVar2 = false;
  }
  if (((*(char *)(param_1 + 0x365) != '\0') && (*(char *)(param_1 + 0x368) == '\0')) &&
     (*(int *)(param_1 + 0x33c) == 1)) {
    bVar2 = false;
  }
  if (((*(char *)(param_1 + 0x366) != '\0') && (*(char *)(param_1 + 0x368) == '\0')) &&
     (*(int *)(param_1 + 0x33c) == 2)) {
    bVar2 = false;
  }
  if (((*(char *)(param_1 + 0x367) != '\0') && (*(char *)(param_1 + 0x368) != '\0')) &&
     ((*(int *)(param_1 + 0x33c) == 2 || (*(int *)(param_1 + 0x33c) == 1)))) {
    bVar2 = false;
  }
  piVar3 = (int *)(param_1 + 0x18);
  iVar4 = 0x20;
  do {
    iVar1 = *(int *)(param_1 + 0x33c);
    if (*piVar3 == iVar1) {
      if (((((*(int *)(param_1 + 0x340) == 1) && (bVar2)) && (iVar1 != 4)) &&
          ((iVar1 != 3 && (iVar1 != 1)))) && (iVar1 != 2)) {
        bVar2 = false;
      }
      else {
        *piVar3 = 0;
      }
    }
    piVar3 = piVar3 + 1;
    iVar4 = iVar4 + -1;
  } while (iVar4 != 0);
  *(undefined4 *)(param_1 + 0x18 + param_2 * 4) = *(undefined4 *)(param_1 + 0x33c);
  *(undefined4 *)(param_1 + 0x33c) = 0;
  DAT_00870768 = 0;
  return;
}


