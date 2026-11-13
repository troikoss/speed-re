// Original UG2 PC v1.2 0x7550d0, 78 bytes. BEHAVIOUR-VERIFIED ONLY (6 clean trials, 34 both-fault):
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



int FUN_007550d0(int *param_1,int param_2)

{
  int *piVar1;
  int *piVar2;
  int *piVar3;
  int iVar4;
  
  iVar4 = param_2;
  if (param_2 < 0) {
    iVar4 = -param_2;
  }
  piVar1 = *(int **)(param_1[5] + (iVar4 % param_1[4]) * 4);
  piVar3 = (int *)(param_1[5] + (iVar4 % param_1[4]) * 4);
  while( true ) {
    if (piVar1 == (int *)0x0) {
      return 0;
    }
    piVar2 = (int *)*piVar3;
    if (piVar2[2] == param_2) break;
    piVar1 = (int *)*piVar2;
    piVar3 = piVar2;
  }
  *piVar3 = *piVar1;
  *piVar1 = param_1[7];
  param_1[7] = (int)piVar1;
  *param_1 = *param_1 + -1;
  return piVar1[1];
}


