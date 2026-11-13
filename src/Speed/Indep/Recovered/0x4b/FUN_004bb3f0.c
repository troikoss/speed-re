// Original UG2 PC v1.2 0x4bb3f0, 92 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_004ffa80();
int __cdecl FUN_00540a60();
int __cdecl FUN_00558540();
int __stdcall FUN_004bb0f0(int);
#define DAT_00783fe5 (*(undefined4 *)0x00783fe5)

void __fastcall FUN_004bb3f0(int param_1)

{
  undefined4 uVar1;
  int iVar2;
  undefined4 uVar3;
  
  *(undefined4 *)(param_1 + 0x100) = 0;
  iVar2 = FUN_004bb0f0(0);
  if (iVar2 < 0) {
    if (*(int *)(param_1 + 0x110) != 0) {
      FUN_00540a60(*(int *)(param_1 + 0x110));
    }
    *(undefined4 *)(param_1 + 0x110) = 0;
    uVar1 = *(undefined4 *)(param_1 + 4);
    uVar3 = FUN_004ffa80(0x8173fc6b);
    FUN_00558540(uVar1,&DAT_00783fe5,0x639,0xffffffff,0xffffffff,uVar3);
  }
  return;
}


