// Original UG2 PC v1.2 0x5db680, 84 bytes. BEHAVIOUR-VERIFIED ONLY (27 clean trials, 13 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.


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



int * __fastcall FUN_005db680(void *_this,int _edx_unused,int param_1)

{
  int *piVar1;
  int iVar2;
  int *piVar3;
  
  piVar1 = *(int **)((int)_this + param_1 * 4 + 0x50);
  if (piVar1 != (int *)0x0) {
    *piVar1 = *piVar1 + 1;
    return piVar1;
  }
  piVar1 = *(int **)((int)_this + 0x6414);
  *(int *)((int)_this + 0x2c10) = *(int *)((int)_this + 0x2c10) + 1;
  iVar2 = *piVar1;
  piVar3 = (int *)piVar1[1];
  *piVar3 = iVar2;
  *(int **)(iVar2 + 4) = piVar3;
  piVar3 = piVar1;
  for (iVar2 = 7; iVar2 != 0; iVar2 = iVar2 + -1) {
    *piVar3 = 0;
    piVar3 = piVar3 + 1;
  }
  *(int **)((int)_this + param_1 * 4 + 0x50) = piVar1;
  *piVar1 = *piVar1 + 1;
  return piVar1;
}


