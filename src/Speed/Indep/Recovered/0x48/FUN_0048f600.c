// Original UG2 PC v1.2 0x48f600, 93 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0048a6a0();
int __cdecl FUN_0048ed60();


undefined4
FUN_0048f600(undefined4 *param_1,int param_2,int param_3,undefined4 param_4,undefined4 param_5,
            undefined4 param_6)

{
  int iVar1;
  undefined4 *puVar2;
  
  if (((param_1 == (undefined4 *)0x0) || (param_3 == 0)) || (param_2 == 0)) {
    return 0;
  }
  puVar2 = param_1;
  for (iVar1 = 0x70; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar2 = 0;
    puVar2 = puVar2 + 1;
  }
  FUN_0048a6a0(param_1,param_3,param_4,param_5);
  FUN_0048ed60(param_1,param_2,param_3 + 0x30,param_3,param_4,param_6);
  return 1;
}


