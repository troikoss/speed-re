// Original UG2 PC v1.2 0x582d80, 144 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0073eae0();
int __cdecl FUN_0073eb70();
int __cdecl FUN_0073eee0();
int __cdecl FUN_0073ef30();
int __cdecl FUN_0073f940();


void __fastcall FUN_00582d80(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  
  iVar3 = 0;
  *(undefined4 *)(param_1 + 0x598) = 0;
  *(undefined4 *)(param_1 + 0x59c) = 0;
  *(undefined4 *)(param_1 + 0x5a0) = 0;
  iVar1 = FUN_0073ef30(*(undefined4 *)(param_1 + 0xf8));
  if (0 < iVar1) {
    do {
      uVar2 = FUN_0073eee0(*(undefined4 *)(param_1 + 0xf8),iVar3);
      iVar1 = FUN_0073eb70(uVar2);
      if (iVar1 != 0) {
        *(int *)(param_1 + 0x598) = *(int *)(param_1 + 0x598) + 1;
      }
      iVar1 = FUN_0073eae0(uVar2);
      if (iVar1 != 0) {
        *(int *)(param_1 + 0x5a0) = *(int *)(param_1 + 0x5a0) + 1;
      }
      iVar1 = FUN_0073f940(uVar2);
      if (iVar1 != 0) {
        *(int *)(param_1 + 0x59c) = *(int *)(param_1 + 0x59c) + 1;
      }
      iVar3 = iVar3 + 1;
      iVar1 = FUN_0073ef30(*(undefined4 *)(param_1 + 0xf8));
    } while (iVar3 < iVar1);
  }
  return;
}


