// Original UG2 PC v1.2 0x4b8f40, 88 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_00558750();
#define DAT_00783fe5 (*(undefined4 *)0x00783fe5)

void __fastcall FUN_004b8f40(int *param_1)

{
  undefined4 uVar1;
  undefined4 uVar2;
  int iVar3;
  
  if (*param_1 != 0) {
    FUN_00540a60(*param_1);
    *param_1 = 0;
  }
  uVar1 = *(undefined4 *)(param_1[2] + 4);
  uVar2 = FUN_004ffa80(0xd857ce14);
  iVar3 = FUN_00558750(uVar1,&DAT_00783fe5,0xf770,0x61c,0x2c0e485a,0xca4c3140,0xca4c3140,1,uVar2);
  *param_1 = iVar3;
  return;
}


