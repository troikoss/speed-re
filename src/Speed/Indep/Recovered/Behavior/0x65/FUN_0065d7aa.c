// Original UG2 PC v1.2 0x65d7aa, 389 bytes. BEHAVIOUR-VERIFIED ONLY (23 clean trials, 17 both-fault):
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



undefined4 __stdcall FUN_0065d7aa(int *param_1,int *param_2,int *param_3,int *param_4)

{
  uint *puVar1;
  int iVar2;
  int *piVar3;
  int *piVar4;
  undefined4 *puVar5;
  uint uVar6;
  undefined4 local_10;
  int *local_c;
  uint local_8;
  
  local_10 = 0;
  iVar2 = *param_1;
  puVar1 = (uint *)(param_1 + 6);
  local_8 = 4;
  if ((uint)param_1[5] < 5) {
    local_8 = param_1[5];
  }
  param_1 = (int *)0x4;
  if ((int *)*puVar1 < (int *)0x5) {
    param_1 = (int *)*puVar1;
  }
  if (iVar2 == 1) {
    if (param_4 != (int *)0x0) {
      local_c = param_4;
      do {
        if ((undefined4 *)*param_3 != (undefined4 *)0x0) {
          piVar4 = (int *)0x0;
          puVar5 = (undefined4 *)*param_3;
          for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
            *puVar5 = 0;
            puVar5 = puVar5 + 1;
          }
          if (param_1 != (int *)0x0) {
            param_4 = param_2;
            do {
              if (local_8 != 0) {
                iVar2 = (int)piVar4 << 2;
                piVar3 = param_4;
                uVar6 = local_8;
                do {
                  *(float *)(iVar2 + *param_3) = (float)(*piVar3 != 0);
                  iVar2 = iVar2 + 0x10;
                  piVar3 = piVar3 + 1;
                  uVar6 = uVar6 - 1;
                } while (uVar6 != 0);
              }
              param_4 = param_4 + 4;
              piVar4 = (int *)((int)piVar4 + 1);
            } while (piVar4 < param_1);
          }
        }
        param_2 = param_2 + 0x10;
        param_3 = param_3 + 1;
        local_c = (int *)((int)local_c + -1);
      } while (local_c != (int *)0x0);
    }
  }
  else if (iVar2 == 2) {
    if (param_4 != (int *)0x0) {
      local_c = param_4;
      do {
        if ((undefined4 *)*param_3 != (undefined4 *)0x0) {
          piVar4 = (int *)0x0;
          puVar5 = (undefined4 *)*param_3;
          for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
            *puVar5 = 0;
            puVar5 = puVar5 + 1;
          }
          if (param_1 != (int *)0x0) {
            param_4 = param_2;
            do {
              if (local_8 != 0) {
                iVar2 = (int)piVar4 << 2;
                piVar3 = param_4;
                uVar6 = local_8;
                do {
                  *(float *)(iVar2 + *param_3) = (float)*piVar3;
                  iVar2 = iVar2 + 0x10;
                  piVar3 = piVar3 + 1;
                  uVar6 = uVar6 - 1;
                } while (uVar6 != 0);
              }
              param_4 = param_4 + 4;
              piVar4 = (int *)((int)piVar4 + 1);
            } while (piVar4 < param_1);
          }
        }
        param_2 = param_2 + 0x10;
        param_3 = param_3 + 1;
        local_c = (int *)((int)local_c + -1);
      } while (local_c != (int *)0x0);
    }
  }
  else if (iVar2 == 3) {
    if (param_4 != (int *)0x0) {
      local_c = param_4;
      do {
        if ((undefined4 *)*param_3 != (undefined4 *)0x0) {
          piVar4 = (int *)0x0;
          puVar5 = (undefined4 *)*param_3;
          for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
            *puVar5 = 0;
            puVar5 = puVar5 + 1;
          }
          if (param_1 != (int *)0x0) {
            param_4 = param_2;
            do {
              if (local_8 != 0) {
                iVar2 = (int)piVar4 << 2;
                piVar3 = param_4;
                uVar6 = local_8;
                do {
                  *(int *)(iVar2 + *param_3) = *piVar3;
                  iVar2 = iVar2 + 0x10;
                  piVar3 = piVar3 + 1;
                  uVar6 = uVar6 - 1;
                } while (uVar6 != 0);
              }
              param_4 = param_4 + 4;
              piVar4 = (int *)((int)piVar4 + 1);
            } while (piVar4 < param_1);
          }
        }
        param_2 = param_2 + 0x10;
        param_3 = param_3 + 1;
        local_c = (int *)((int)local_c + -1);
      } while (local_c != (int *)0x0);
    }
  }
  else {
    local_10 = 0x8876086c;
  }
  return local_10;
}


