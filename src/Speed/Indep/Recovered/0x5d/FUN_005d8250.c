// Original UG2 PC v1.2 0x5d8250, 112 bytes. Auto-matched: byte-identical to speed2.exe under
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



void __fastcall FUN_005d8250(int param_1,int _edx_unused,int param_2)

{
  int iVar1;
  int *piVar2;
  
  piVar2 = (int *)(param_2 + 8);
  if (piVar2 < (int *)(*(int *)(param_2 + 4) + 8 + param_2)) {
    do {
      iVar1 = *piVar2;
      if (iVar1 != 0x34131) {
        if (iVar1 == 0x34134) {
          *(undefined4 *)(param_1 + 8) = 0;
          *(undefined4 *)(param_1 + 0xc) = 0;
        }
        else if (iVar1 == 0x34135) {
          *(undefined4 *)(param_1 + 0x10) = 0;
          *(undefined4 *)(param_1 + 0x14) = 0;
        }
        else if (iVar1 == 0x34136) {
          *(undefined4 *)(param_1 + 0x18) = 0;
          *(undefined4 *)(param_1 + 0x1c) = 0;
        }
        else if (iVar1 == 0x34137) {
          *(undefined4 *)(param_1 + 0x20) = 0;
          *(undefined4 *)(param_1 + 0x24) = 0;
        }
      }
      piVar2 = (int *)((int)piVar2 + piVar2[1] + 8);
    } while (piVar2 < (int *)(*(int *)(param_2 + 4) + 8 + param_2));
  }
  return;
}


