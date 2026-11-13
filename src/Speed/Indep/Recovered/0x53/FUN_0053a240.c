// Original UG2 PC v1.2 0x53a240, 43 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_00440ef0();
#define PTR_FUN_0079be30 (*(code *)0x0079be30)

void __fastcall FUN_0053a240(undefined4 *param_1)

{
  int *piVar1;
  int *piVar2;
  int iVar3;
  int *piVar4;
  
  piVar1 = param_1 + 1;
  *param_1 = &PTR_FUN_0079be30;
  piVar2 = (int *)*piVar1;
  while (piVar2 != piVar1) {
    iVar3 = *piVar2;
    piVar4 = (int *)piVar2[1];
    *piVar4 = iVar3;
    *(int **)(iVar3 + 4) = piVar4;
    FUN_00440ef0(piVar2);
    piVar2 = (int *)*piVar1;
  }
  return;
}


