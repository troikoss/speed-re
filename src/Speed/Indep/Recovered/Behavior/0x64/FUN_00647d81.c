// Original UG2 PC v1.2 0x647d81, 225 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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



int __stdcall FUN_00647d81(int param_1,int *param_2,int *param_3)

{
  int iVar1;
  int iVar2;
  int *piVar3;
  int *piVar4;
  int iVar5;
  int *piVar6;
  int iVar7;
  int iVar8;
  int local_8;
  
  piVar3 = param_3;
  piVar6 = param_2;
  iVar5 = *param_2;
  piVar4 = (int *)(param_1 + iVar5);
  iVar8 = piVar4[4];
  iVar1 = piVar4[1];
  if (iVar8 == 0) {
    iVar8 = 1;
  }
  if (iVar1 < 0) {
LAB_00647e59:
    iVar8 = 0;
  }
  else {
    if (iVar1 < 4) {
      iVar1 = piVar4[6];
      iVar2 = piVar4[5];
      iVar7 = *piVar4;
      if (((iVar7 == 1) || (iVar7 == 2)) || (iVar7 == 3)) {
        iVar7 = 4;
      }
      else {
        iVar7 = 0;
        iVar8 = 0;
      }
      *param_2 = iVar5 + 0x1c;
      piVar6 = (int *)(iVar1 * iVar2 * iVar7);
      *param_3 = iVar1 * iVar2 * iVar8;
    }
    else if (iVar1 == 4) {
      iVar1 = *piVar4;
      if ((iVar1 < 10) || ((0xe < iVar1 && (iVar1 != 0xf30000)))) {
        piVar6 = (int *)0x4;
      }
      else {
        piVar6 = (int *)0x0;
      }
      *param_2 = iVar5 + 0x14;
      *param_3 = iVar8;
    }
    else {
      if (iVar1 != 5) goto LAB_00647e59;
      piVar4 = (int *)piVar4[5];
      param_3 = (int *)0x0;
      *param_2 = iVar5 + 0x18;
      *piVar3 = 0;
      if (piVar4 != (int *)0x0) {
        param_2 = piVar4;
        do {
          iVar5 = FUN_00647d81(param_1,piVar6,&local_8);
          param_3 = (int *)((int)param_3 + iVar5);
          *piVar3 = *piVar3 + local_8;
          param_2 = (int *)((int)param_2 + -1);
        } while (param_2 != (int *)0x0);
      }
      *piVar3 = *piVar3 * iVar8;
      piVar6 = param_3;
    }
    iVar8 = (int)piVar6 * iVar8;
  }
  return iVar8;
}


