// Original UG2 PC v1.2 0x4a0890, 143 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_00582530();


undefined4 __stdcall FUN_004a0890(undefined4 param_1)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = FUN_00582530(param_1);
  if (iVar1 == 0) {
    uVar2 = FUN_004ffa80(0x719afe9e);
    return uVar2;
  }
  if (iVar1 == 1) {
    uVar2 = FUN_004ffa80(0xf600c82b);
    return uVar2;
  }
  if (iVar1 == 3) {
    uVar2 = FUN_004ffa80(0x36c5fae4);
    return uVar2;
  }
  if (iVar1 == 2) {
    uVar2 = FUN_004ffa80(0xe8140569);
    return uVar2;
  }
  if (iVar1 == 4) {
    uVar2 = FUN_004ffa80(0xb24883a);
    return uVar2;
  }
  if (iVar1 == 5) {
    uVar2 = FUN_004ffa80(0x8cb6e51e);
    return uVar2;
  }
  return 0;
}


