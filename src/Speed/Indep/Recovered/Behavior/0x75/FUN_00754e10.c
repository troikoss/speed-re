// Original UG2 PC v1.2 0x754e10, 83 bytes. BEHAVIOUR-VERIFIED ONLY (19 clean trials, 21 both-fault):
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



int FUN_00754e10(int *param_1)

{
  int iVar1;
  int *piVar2;
  int iVar3;
  
  if (*param_1 == 0) {
    return 0;
  }
  iVar1 = param_1[5];
  if (*(int *)(iVar1 + param_1[1] * 4) == 0) {
    do {
      iVar3 = (param_1[1] + 1) % param_1[4];
      param_1[1] = iVar3;
    } while (*(int *)(param_1[5] + iVar3 * 4) == 0);
  }
  piVar2 = *(int **)(iVar1 + param_1[1] * 4);
  *(int *)(iVar1 + param_1[1] * 4) = *piVar2;
  *piVar2 = param_1[7];
  param_1[7] = (int)piVar2;
  *param_1 = *param_1 + -1;
  return piVar2[1];
}


