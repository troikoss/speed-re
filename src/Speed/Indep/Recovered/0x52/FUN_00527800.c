// Original UG2 PC v1.2 0x527800, 134 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_004ffe60();
int __cdecl FUN_004fffc0();
int __cdecl FUN_00512a10();
int __stdcall FUN_00527780(int);


void __stdcall FUN_00527800(int *param_1,int param_2)

{
  int iVar1;
  
  *param_1 = 0;
  param_1[1] = 0;
  param_1[2] = 0;
  param_1[4] = 0;
  param_1[3] = 0;
  iVar1 = FUN_00527780(param_2);
  if (iVar1 != 0) {
    *param_1 = param_2;
    param_1[1] = iVar1;
    return;
  }
  if (param_2 != 0) {
    iVar1 = FUN_004ffe60(*(undefined4 *)(param_2 + 0x30));
    if (iVar1 != 0) {
      *param_1 = param_2;
      param_1[2] = iVar1;
      return;
    }
  }
  iVar1 = FUN_004fffc0(param_2);
  if (iVar1 != 0) {
    *param_1 = param_2;
    param_1[3] = iVar1;
    return;
  }
  if (param_2 != 0) {
    iVar1 = FUN_00512a10(*(undefined4 *)(param_2 + 0x30));
    if (iVar1 != 0) {
      param_1[4] = iVar1;
      *param_1 = param_2;
    }
  }
  return;
}


