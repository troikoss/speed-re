// Original UG2 PC v1.2 0x6886bf, 195 bytes. BEHAVIOUR-VERIFIED ONLY (25 clean trials, 15 both-fault):
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



int __stdcall FUN_006886bf(int param_1,int *param_2,int *param_3)

{
  uint uVar1;
  int iVar2;
  int *piVar3;
  int iVar4;
  uint *puVar5;
  uint *puVar6;
  
  puVar6 = (uint *)param_2;
  iVar4 = param_1;
  puVar5 = (uint *)param_3;
  if (param_3 == (int *)0x0) {
    puVar5 = (uint *)&param_1;
  }
  iVar2 = *(int *)(param_1 + 4);
  if (iVar2 == 9) {
    iVar4 = *(int *)(param_1 + 0x10);
    uVar1 = 1;
    puVar6 = puVar5;
    if ((iVar4 == 1) || (iVar4 == 0)) {
      *param_2 = 1;
      uVar1 = *(uint *)(param_1 + 0x1c);
    }
    else if (iVar4 == 2) {
      *param_2 = *(int *)(param_1 + 0x1c);
      uVar1 = *(uint *)(param_1 + 0x18);
    }
    else {
      if (iVar4 != 3) {
        return -0x7fffbffb;
      }
      *param_2 = 1;
    }
  }
  else {
    if (iVar2 != 8) {
      if (iVar2 != 1) {
        return -0x7fffbffb;
      }
      *param_2 = 0;
      *puVar5 = 0;
      do {
        iVar2 = FUN_006886bf(*(int *)(*(int *)(*(int *)(iVar4 + 8) + 0x18) + 0x20),(int *)&param_3,
                             (int *)&param_2);
        if (iVar2 < 0) {
          return iVar2;
        }
        *puVar6 = *puVar6 + (int)param_3;
        piVar3 = (int *)*puVar5;
        if ((int *)*puVar5 < param_2) {
          piVar3 = param_2;
        }
        *puVar5 = (uint)piVar3;
        iVar4 = *(int *)(iVar4 + 0xc);
      } while (iVar4 != 0);
      return 0;
    }
    iVar2 = FUN_006886bf(*(int *)(param_1 + 0x10),param_2,(int *)puVar5);
    if (iVar2 < 0) {
      return iVar2;
    }
    uVar1 = *(int *)(iVar4 + 0x14) * *puVar6;
  }
  *puVar6 = uVar1;
  return 0;
}


