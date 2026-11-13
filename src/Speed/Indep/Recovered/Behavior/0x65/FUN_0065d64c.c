// Original UG2 PC v1.2 0x65d64c, 350 bytes. BEHAVIOUR-VERIFIED ONLY (23 clean trials, 17 both-fault):
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



undefined4 __stdcall FUN_0065d64c(int *param_1,int param_2,int *param_3,int param_4)

{
  uint *puVar1;
  uint *puVar2;
  int iVar3;
  uint uVar4;
  undefined4 *puVar5;
  int *piVar6;
  undefined4 local_c;
  uint local_8;
  
  iVar3 = *param_1;
  puVar1 = (uint *)(param_1 + 5);
  puVar2 = (uint *)(param_1 + 6);
  local_c = 0;
  param_1 = (int *)0x4;
  if ((int *)*puVar2 < (int *)0x5) {
    param_1 = (int *)*puVar2;
  }
  local_8 = 4;
  if (*puVar1 < 5) {
    local_8 = *puVar1;
  }
  if (iVar3 == 1) {
    if (param_4 != 0) {
      do {
        if ((undefined4 *)*param_3 != (undefined4 *)0x0) {
          puVar5 = (undefined4 *)*param_3;
          for (iVar3 = 0x10; iVar3 != 0; iVar3 = iVar3 + -1) {
            *puVar5 = 0;
            puVar5 = puVar5 + 1;
          }
          piVar6 = (int *)0x0;
          if (param_1 != (int *)0x0) {
            do {
              if (local_8 != 0) {
                iVar3 = (int)piVar6 << 4;
                uVar4 = local_8;
                do {
                  *(float *)(iVar3 + *param_3) = (float)(*(int *)(iVar3 + param_2) != 0);
                  iVar3 = iVar3 + 4;
                  uVar4 = uVar4 - 1;
                } while (uVar4 != 0);
              }
              piVar6 = (int *)((int)piVar6 + 1);
            } while (piVar6 < param_1);
          }
        }
        param_2 = param_2 + 0x40;
        param_3 = param_3 + 1;
        param_4 = param_4 + -1;
      } while (param_4 != 0);
    }
  }
  else if (iVar3 == 2) {
    if (param_4 != 0) {
      do {
        if ((undefined4 *)*param_3 != (undefined4 *)0x0) {
          puVar5 = (undefined4 *)*param_3;
          for (iVar3 = 0x10; iVar3 != 0; iVar3 = iVar3 + -1) {
            *puVar5 = 0;
            puVar5 = puVar5 + 1;
          }
          piVar6 = (int *)0x0;
          if (param_1 != (int *)0x0) {
            do {
              if (local_8 != 0) {
                iVar3 = (int)piVar6 << 4;
                uVar4 = local_8;
                do {
                  *(float *)(iVar3 + *param_3) = (float)*(int *)(iVar3 + param_2);
                  iVar3 = iVar3 + 4;
                  uVar4 = uVar4 - 1;
                } while (uVar4 != 0);
              }
              piVar6 = (int *)((int)piVar6 + 1);
            } while (piVar6 < param_1);
          }
        }
        param_2 = param_2 + 0x40;
        param_3 = param_3 + 1;
        param_4 = param_4 + -1;
      } while (param_4 != 0);
    }
  }
  else if (iVar3 == 3) {
    if (param_4 != 0) {
      do {
        if ((undefined4 *)*param_3 != (undefined4 *)0x0) {
          puVar5 = (undefined4 *)*param_3;
          for (iVar3 = 0x10; iVar3 != 0; iVar3 = iVar3 + -1) {
            *puVar5 = 0;
            puVar5 = puVar5 + 1;
          }
          piVar6 = (int *)0x0;
          if (param_1 != (int *)0x0) {
            do {
              if (local_8 != 0) {
                iVar3 = (int)piVar6 << 4;
                uVar4 = local_8;
                do {
                  *(undefined4 *)(iVar3 + *param_3) = *(undefined4 *)(iVar3 + param_2);
                  iVar3 = iVar3 + 4;
                  uVar4 = uVar4 - 1;
                } while (uVar4 != 0);
              }
              piVar6 = (int *)((int)piVar6 + 1);
            } while (piVar6 < param_1);
          }
        }
        param_2 = param_2 + 0x40;
        param_3 = param_3 + 1;
        param_4 = param_4 + -1;
      } while (param_4 != 0);
    }
  }
  else {
    local_c = 0x8876086c;
  }
  return local_c;
}


