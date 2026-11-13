// Original UG2 PC v1.2 0x4355d0, 131 bytes. Auto-matched: byte-identical to speed2.exe under
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

#define _DAT_00827d34 (*(undefined4 *)0x00827d34)
#define _DAT_00827c5c (*(undefined4 *)0x00827c5c)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __stdcall FUN_004355d0(int param_1, int param_2, int _pad0)

{
  undefined4 *puVar1;
  int iVar2;
  int *piVar3;
  
  if (param_2 == _DAT_00827d34) {
    piVar3 = *(int **)(param_1 + 0x14);
    if (piVar3 != (int *)(param_1 + 0x14)) {
      do {
        if (*(int *)(piVar3[2] + 0x14) != 0) {
          *(undefined4 *)(*(int *)(piVar3[2] + 0x14) + 0x10) = 1;
        }
        piVar3 = (int *)*piVar3;
      } while (piVar3 != (int *)(param_1 + 0x14));
      return;
    }
  }
  else if (param_2 == _DAT_00827c5c) {
    for (puVar1 = *(undefined4 **)(param_1 + 0x14); puVar1 != (undefined4 *)(param_1 + 0x14);
        puVar1 = (undefined4 *)*puVar1) {
      iVar2 = *(int *)(puVar1[2] + 0x14);
      if (iVar2 != 0) {
        if (*(int *)(iVar2 + 0x10) == 2) {
          *(undefined4 *)(iVar2 + 0x10) = 1;
        }
        else {
          *(uint *)(iVar2 + 8) = (uint)(*(int *)(iVar2 + 8) == 0);
        }
      }
    }
  }
  return;
}


