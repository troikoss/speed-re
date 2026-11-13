// Original UG2 PC v1.2 0x4985a0, 95 bytes. Auto-matched: byte-identical to speed2.exe under
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
typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef void *HANDLE;
typedef unsigned int size_t;
typedef int BOOL;
int __cdecl FUN_0057ef60();
int __cdecl FUN_00580e90();
#define PTR_FUN_0078f8c8 (*(code *)0x0078f8c8)
#define _DAT_00836344 (*(undefined4 *)0x00836344)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004985a0(undefined4 *param_1)

{
  char cVar1;
  
  *param_1 = &PTR_FUN_0078f8c8;
  _DAT_00836344 = 0;
  if (param_1[5] != 0) {
    cVar1 = *(char *)(param_1 + 6);
    while (cVar1 == '\0') {
      FUN_0057ef60();
      cVar1 = *(char *)(param_1 + 6);
    }
  }
  if ((int *)param_1[5] != (int *)0x0) {
    FUN_00580e90((int *)param_1[5]);
  }
  if ((int *)param_1[8] != (int *)0x0) {
    FUN_00580e90((int *)param_1[8]);
  }
  if ((int *)param_1[9] != (int *)0x0) {
    FUN_00580e90((int *)param_1[9]);
  }
  return;
}


