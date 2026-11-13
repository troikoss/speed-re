// Original UG2 PC v1.2 0x755020, 110 bytes. BEHAVIOUR-VERIFIED ONLY (7 clean trials, 33 both-fault):
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
int __cdecl FUN_00754db0();


undefined4 FUN_00755020(int *param_1,int param_2,int param_3)

{
  int iVar1;
  int *piVar2;
  int iVar3;
  int iVar4;
  
  iVar3 = param_2;
  if (param_2 < 0) {
    iVar3 = -param_2;
  }
  iVar1 = param_1[4];
  if (param_1[7] == 0) {
    iVar4 = FUN_00754db0((int)param_1,(*param_1 * 3 + 10) / 2);
    if (iVar4 < 1) {
      return 0xffffffff;
    }
  }
  piVar2 = (int *)param_1[7];
  param_1[7] = *piVar2;
  piVar2[2] = param_2;
  piVar2[1] = param_3;
  *piVar2 = *(int *)(param_1[5] + (iVar3 % iVar1) * 4);
  *(int **)(param_1[5] + (iVar3 % iVar1) * 4) = piVar2;
  *param_1 = *param_1 + 1;
  return 1;
}


