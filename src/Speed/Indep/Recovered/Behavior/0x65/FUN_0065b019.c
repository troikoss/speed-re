// Original UG2 PC v1.2 0x65b019, 127 bytes. BEHAVIOUR-VERIFIED ONLY (15 clean trials, 25 both-fault):
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



int __stdcall FUN_0065b019(int *param_1,uint param_2)

{
  int *piVar1;
  uint uVar2;
  int *piVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  
  piVar3 = param_1;
  iVar4 = *param_1;
  iVar5 = *(int *)(iVar4 + 0x10);
  iVar6 = *(int *)(iVar4 + 4);
  if (iVar5 == 0) {
    iVar5 = 1;
  }
  if (-1 < iVar6) {
    if (iVar6 < 4) {
      uVar2 = *(uint *)(iVar4 + 0x14);
      iVar6 = *(int *)(iVar4 + 0x18);
      *param_1 = iVar4 + 0x1c;
      return ((uint)(uVar2 % param_2 != 0) + uVar2 / param_2) * iVar6 * iVar5;
    }
    if ((iVar6 != 4) && (iVar6 == 5)) {
      piVar1 = *(int **)(iVar4 + 0x14);
      iVar6 = 0;
      *param_1 = iVar4 + 0x18;
      if (piVar1 != (int *)0x0) {
        param_1 = piVar1;
        do {
          iVar4 = FUN_0065b019(piVar3,param_2);
          iVar6 = iVar6 + iVar4;
          param_1 = (int *)((int)param_1 + -1);
        } while (param_1 != (int *)0x0);
      }
      return iVar6 * iVar5;
    }
  }
  return 0;
}


