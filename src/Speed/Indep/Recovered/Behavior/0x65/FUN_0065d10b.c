// Original UG2 PC v1.2 0x65d10b, 383 bytes. BEHAVIOUR-VERIFIED ONLY (23 clean trials, 17 both-fault):
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



undefined4 __stdcall FUN_0065d10b(int *param_1,int param_2,float *param_3,uint param_4)

{
  uint *puVar1;
  uint *puVar2;
  float *pfVar3;
  float *pfVar4;
  int iVar5;
  uint uVar6;
  float *pfVar7;
  int *piVar8;
  undefined4 local_c;
  uint local_8;
  
  pfVar7 = param_3;
  iVar5 = *param_1;
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
  if (iVar5 == 1) {
    for (iVar5 = (param_4 & 0x3ffffff) << 4; iVar5 != 0; iVar5 = iVar5 + -1) {
      *param_3 = 0.0;
      param_3 = param_3 + 1;
    }
    for (iVar5 = 0; iVar5 != 0; iVar5 = iVar5 + -1) {
      *(undefined1 *)param_3 = 0;
      param_3 = (float *)((int)param_3 + 1);
    }
    if (param_4 != 0) {
      param_3 = (float *)param_4;
      do {
        if (param_1 != (int *)0x0) {
          param_4 = (uint)param_1;
          pfVar3 = pfVar7;
          do {
            if (local_8 != 0) {
              pfVar4 = pfVar3;
              uVar6 = local_8;
              do {
                *pfVar4 = (float)(*(int *)((int)pfVar4 + (param_2 - (int)pfVar7)) != 0);
                pfVar4 = pfVar4 + 1;
                uVar6 = uVar6 - 1;
              } while (uVar6 != 0);
            }
            pfVar3 = pfVar3 + 4;
            param_4 = param_4 - 1;
          } while (param_4 != 0);
        }
        param_2 = param_2 + 0x40;
        pfVar7 = pfVar7 + 0x10;
        param_3 = (float *)((int)param_3 + -1);
      } while (param_3 != (float *)0x0);
    }
  }
  else if (iVar5 == 2) {
    for (iVar5 = (param_4 & 0x3ffffff) << 4; iVar5 != 0; iVar5 = iVar5 + -1) {
      *pfVar7 = 0.0;
      pfVar7 = pfVar7 + 1;
    }
    for (iVar5 = 0; iVar5 != 0; iVar5 = iVar5 + -1) {
      *(undefined1 *)pfVar7 = 0;
      pfVar7 = (float *)((int)pfVar7 + 1);
    }
    if (param_4 != 0) {
      do {
        if (param_1 != (int *)0x0) {
          pfVar7 = param_3;
          piVar8 = param_1;
          do {
            if (local_8 != 0) {
              pfVar3 = pfVar7;
              uVar6 = local_8;
              do {
                *pfVar3 = (float)*(int *)((int)pfVar3 + (param_2 - (int)param_3));
                pfVar3 = pfVar3 + 1;
                uVar6 = uVar6 - 1;
              } while (uVar6 != 0);
            }
            pfVar7 = pfVar7 + 4;
            piVar8 = (int *)((int)piVar8 + -1);
          } while (piVar8 != (int *)0x0);
        }
        param_2 = param_2 + 0x40;
        param_3 = param_3 + 0x10;
        param_4 = param_4 - 1;
      } while (param_4 != 0);
    }
  }
  else if (iVar5 == 3) {
    for (iVar5 = (param_4 & 0x3ffffff) << 4; iVar5 != 0; iVar5 = iVar5 + -1) {
      *pfVar7 = 0.0;
      pfVar7 = pfVar7 + 1;
    }
    for (iVar5 = 0; iVar5 != 0; iVar5 = iVar5 + -1) {
      *(undefined1 *)pfVar7 = 0;
      pfVar7 = (float *)((int)pfVar7 + 1);
    }
    if (param_4 != 0) {
      do {
        if (param_1 != (int *)0x0) {
          pfVar7 = param_3;
          piVar8 = param_1;
          do {
            if (local_8 != 0) {
              pfVar3 = pfVar7;
              uVar6 = local_8;
              do {
                *pfVar3 = *(float *)((param_2 - (int)param_3) + (int)pfVar3);
                pfVar3 = pfVar3 + 1;
                uVar6 = uVar6 - 1;
              } while (uVar6 != 0);
            }
            pfVar7 = pfVar7 + 4;
            piVar8 = (int *)((int)piVar8 + -1);
          } while (piVar8 != (int *)0x0);
        }
        param_2 = param_2 + 0x40;
        param_3 = param_3 + 0x10;
        param_4 = param_4 - 1;
      } while (param_4 != 0);
    }
  }
  else {
    local_c = 0x8876086c;
  }
  return local_c;
}


