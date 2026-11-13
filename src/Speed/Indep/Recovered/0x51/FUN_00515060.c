// Original UG2 PC v1.2 0x515060, 105 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_004ffc50();
int __cdecl FUN_004ffe20();
int __cdecl FUN_004fff80();


void __fastcall FUN_00515060(int param_1,int _edx_unused,undefined4 *param_2)

{
  int iVar1;
  undefined4 uVar2;
  
  *param_2 = 0;
  param_2[1] = 0;
  param_2[2] = 0;
  param_2[3] = 0;
  param_2[4] = 0;
  if (*(int *)(param_1 + 8) != 0) {
    uVar2 = *(undefined4 *)(*(int *)(param_1 + 8) + 0xc);
    iVar1 = FUN_004ffe20(uVar2);
    if (iVar1 != 0) {
      param_2[2] = iVar1;
      uVar2 = FUN_004ffc50(*(undefined4 *)(iVar1 + 0x3c));
      *param_2 = uVar2;
      return;
    }
    iVar1 = FUN_004fff80(uVar2);
    if (iVar1 != 0) {
      param_2[3] = iVar1;
      uVar2 = FUN_004ffc50(*(undefined4 *)(iVar1 + 0x2c));
      *param_2 = uVar2;
    }
  }
  return;
}


