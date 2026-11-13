// Original UG2 PC v1.2 0x493ea0, 126 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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
int __cdecl FUN_00493790();
int __cdecl FUN_005bb720();
#define DAT_0082d380 (*(undefined4 *)0x0082d380)

int * FUN_00493ea0(int *param_1)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  int *piVar4;
  int *piVar5;
  
  if (*param_1 != -0x7fecbfff) {
    return (int *)0x0;
  }
  iVar2 = param_1[1];
  piVar5 = (int *)0x0;
  for (piVar1 = param_1 + 2; piVar1 < (int *)(iVar2 + 8 + (int)param_1);
      piVar1 = (int *)((int)piVar1 + piVar1[1] + 8)) {
    iVar3 = *piVar1;
    if (iVar3 == 0x134002) {
      iVar3 = piVar1[2];
      piVar4 = (int *)piVar1[3];
      piVar5 = piVar1 + 2;
      *piVar4 = iVar3;
      *(int **)(iVar3 + 4) = piVar4;
      FUN_00493790((int)piVar5);
      DAT_0082d380 = DAT_0082d380 + -1;
    }
    else if (((iVar3 != 0x134003) && (iVar3 != 0x134004)) && (iVar3 == -0x7fecbff8)) {
      FUN_005bb720((int)piVar1);
    }
  }
  return piVar5;
}


