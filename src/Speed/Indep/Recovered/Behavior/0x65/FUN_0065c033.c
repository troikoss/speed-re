// Original UG2 PC v1.2 0x65c033, 240 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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



int __stdcall FUN_0065c033(int param_1,int *param_2,int *param_3)

{
  int iVar1;
  int iVar2;
  int *piVar3;
  int *piVar4;
  int iVar5;
  int *piVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  
  piVar4 = param_3;
  piVar3 = param_2;
  iVar1 = *param_2;
  piVar6 = (int *)(iVar1 + param_1);
  iVar8 = piVar6[4];
  iVar5 = piVar6[1];
  if (iVar8 == 0) {
    iVar8 = 1;
  }
  if (iVar5 < 0) {
LAB_0065c11a:
    iVar8 = 0;
  }
  else {
    if (iVar5 < 4) {
      iVar5 = piVar6[5];
      iVar2 = piVar6[6];
      iVar9 = *piVar6;
      if (((iVar9 == 1) || (iVar9 == 2)) || (iVar9 == 3)) {
        iVar9 = 4;
        iVar7 = 4;
      }
      else {
        iVar9 = 0;
        iVar7 = 0;
      }
      *param_2 = iVar1 + 0x1c;
      *param_3 = *param_3 + iVar7 * iVar2 * iVar5 * iVar8;
      piVar6 = (int *)(iVar2 * iVar5 * iVar9);
    }
    else if (iVar5 == 4) {
      if ((*piVar6 < 10) || (0xe < *piVar6)) {
        piVar6 = (int *)0x4;
      }
      else {
        piVar6 = (int *)0x0;
      }
      *param_2 = iVar1 + 0x14;
      *param_3 = *param_3 + iVar8 * 4;
    }
    else {
      if (iVar5 != 5) goto LAB_0065c11a;
      piVar6 = (int *)piVar6[5];
      param_3 = (int *)0x0;
      *param_2 = iVar1 + 0x18;
      iVar1 = *piVar4;
      if (piVar6 != (int *)0x0) {
        param_2 = piVar6;
        do {
          iVar5 = FUN_0065c033(param_1,piVar3,piVar4);
          param_3 = (int *)((int)param_3 + iVar5);
          param_2 = (int *)((int)param_2 + -1);
        } while (param_2 != (int *)0x0);
      }
      *piVar4 = (*piVar4 - iVar1) * (iVar8 + -1) + *piVar4;
      piVar6 = param_3;
    }
    iVar8 = (int)piVar6 * iVar8;
  }
  return iVar8;
}


