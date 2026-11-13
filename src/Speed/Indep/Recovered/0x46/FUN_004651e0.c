// Original UG2 PC v1.2 0x4651e0, 62 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_00465180();
int __cdecl FUN_006f76e9();
int __cdecl FUN_006f76f2();
int __cdecl FUN_006f7c1b();


void FUN_004651e0(undefined4 param_1,undefined4 param_2,int param_3)

{
  int iVar1;
  
  FUN_006f76e9(*(undefined4 *)(param_3 + 0x184));
  iVar1 = FUN_006f7c1b(param_3 + 0x78,1,*(undefined4 *)(param_3 + 0x4c),param_3);
  *(int *)(param_3 + 0x184) = iVar1;
  if (iVar1 != 0) {
    FUN_006f76f2(iVar1,FUN_00465180);
  }
  return;
}


