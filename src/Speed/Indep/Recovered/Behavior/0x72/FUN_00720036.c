// Original UG2 PC v1.2 0x720036, 108 bytes. BEHAVIOUR-VERIFIED ONLY (21 clean trials, 19 both-fault):
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
int __cdecl FUN_00735430();


undefined4 FUN_00720036(int *param_1)

{
  int *piVar1;
  int *piVar2;
  int iVar3;
  int *piVar4;
  int iVar5;
  
  piVar1 = param_1;
  if ((char)param_1[2] == '\0') {
    piVar4 = param_1 + 3;
  }
  else {
    piVar4 = param_1 + (uint)*(byte *)((int)param_1 + 9) * 2 + 3;
    iVar5 = 0;
    piVar2 = piVar4;
    if (*(byte *)((int)param_1 + 9) == 0) goto LAB_0072008c;
    do {
      piVar2 = piVar2 + 1;
      iVar3 = *piVar2;
      if (iVar3 < param_1[iVar5 * 2 + 3]) {
        iVar3 = param_1[iVar5 * 2 + 3];
      }
      else if (param_1[iVar5 * 2 + 4] < iVar3) {
        iVar3 = param_1[iVar5 * 2 + 4];
      }
      *piVar2 = iVar3;
      iVar5 = iVar5 + 1;
    } while (iVar5 < (int)(uint)*(byte *)((int)param_1 + 9));
  }
  param_1 = piVar4;
  piVar4 = param_1;
LAB_0072008c:
  if (*piVar4 != 0) {
    FUN_00735430(piVar1,piVar4 + 1);
  }
  return 0;
}


